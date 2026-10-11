#!/usr/bin/env python3
"""Replace PRGUpdate.exe in the upstream QY8XXX card image."""
import hashlib
import mmap
import struct
import sys

PREAMBLE = 0x200
ARM_OFFSET = 0x14600000
ARM_SLOT_SIZE = 0x3C00000
NB_HEADER_SIZE = 0x200
SH4_BANK_SIZE = 0xA00000
SH4_BANK_SLOTS = (0x03E00000, 0x04800000)
PHYSFIRST = 0x88000000
ROM_SIG = b"ECEC"
EXCLUDED = {0x30, 0x34, 0x38, 0x3C, 0x84, 0x88, 0x8C}


def u16(data, offset):
    return struct.unpack_from("<H", data, offset)[0]


def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def align(value, boundary):
    return (value + boundary - 1) & ~(boundary - 1)


def checksum(data, length):
    total = sum(u32(data, offset) for offset in range(0, 0x90, 4)
                if offset not in EXCLUDED)
    total += sum(struct.unpack_from("<%dI" % ((length - 0x90) // 4),
                                    data, 0x90))
    return total & 0xFFFFFFFF


def parse_pe(exe):
    pe = u32(exe, 0x3C)
    if exe[pe:pe + 4] != b"PE\0\0":
        raise ValueError("input is not a PE image")
    count = u16(exe, pe + 6)
    opt_size = u16(exe, pe + 20)
    opt = pe + 24
    if u16(exe, opt) != 0x10B or u16(exe, opt + 68) != 9:
        raise ValueError("expected Windows CE PE32 executable")
    entry = u32(exe, opt + 16)
    vbase = u32(exe, opt + 28)
    module_size = u32(exe, opt + 56)
    import_rva, import_size = struct.unpack_from("<II", exe, opt + 104)
    table = opt + opt_size
    sections = []
    for index in range(count):
        offset = table + index * 40
        vsize, rva, raw_size, raw = struct.unpack_from("<IIII", exe, offset + 8)
        flags = u32(exe, offset + 36)
        if flags & 0x02000000:
            continue
        if raw + raw_size > len(exe):
            raise ValueError("PE section extends beyond the executable")
        sections.append({"rva": rva, "vsize": vsize, "psize": raw_size,
                         "raw": raw, "flags": flags,
                         "data": exe[raw:raw + raw_size]})
    sections.sort(key=lambda section: section["rva"])

    def rva_to_offset(rva):
        for section in sections:
            span = max(section["vsize"], section["psize"])
            if section["rva"] <= rva < section["rva"] + span:
                offset = section["raw"] + rva - section["rva"]
                if offset >= len(exe):
                    break
                return offset
        raise ValueError("PE RVA 0x%x is not backed by a section" % rva)

    imports = []
    if import_rva:
        descriptor = rva_to_offset(import_rva)
        while True:
            original, _, _, name_rva, first = struct.unpack_from(
                "<5I", exe, descriptor)
            descriptor += 20
            if not name_rva:
                break
            name_offset = rva_to_offset(name_rva)
            end = exe.index(b"\0", name_offset)
            dll = exe[name_offset:end].decode("ascii").lower()
            thunk = rva_to_offset(original or first)
            symbols = []
            while True:
                value = u32(exe, thunk)
                thunk += 4
                if not value:
                    break
                if value & 0x80000000:
                    symbols.append(("ordinal", value & 0xFFFF))
                    continue
                hint = rva_to_offset(value)
                end = exe.index(b"\0", hint + 2)
                symbols.append(("name", exe[hint + 2:end].decode("ascii")))
            imports.append((dll, symbols))
    return sections, entry, vbase, module_size, import_rva, import_size, imports


def rom_modules(img, toc, count):
    modules = {}
    for index in range(count):
        entry = toc + index * 0x20
        name_offset = u32(img, entry + 0x10) - PHYSFIRST
        end = bytes(img).index(b"\0", name_offset)
        name = bytes(img[name_offset:end]).decode("latin1").lower()
        e32 = u32(img, entry + 0x14) - PHYSFIRST
        o32 = u32(img, entry + 0x18) - PHYSFIRST
        sections = [struct.unpack_from("<6I", img, o32 + i * 0x18)
                    for i in range(u16(img, e32))]
        modules[name] = (entry, e32, sections)
    return modules


def rom_rva(img, sections, rva):
    for vsize, va, raw_size, data_va, _, flags in sections:
        if va <= rva < va + vsize:
            if flags & 0x2000:
                return None
            return data_va - PHYSFIRST + rva - va
    raise ValueError("ROM export RVA 0x%x is outside sections" % rva)


def check_imports(img, modules, imports):
    missing = []
    for dll, names in imports:
        dll = dll if dll.endswith(".dll") else dll + ".dll"
        module = modules.get(dll)
        if module is None:
            missing.append(dll)
            continue
        _, e32, sections = module
        export_rva = u32(img, e32 + 0x24)
        if not export_rva:
            # ddraw.dll's exports are compressed in the upstream ROM. The
            # release tool only imports DirectDrawCreate from this DLL.
            if dll == "ddraw.dll" and all(
                    kind == "name" and symbol == "DirectDrawCreate"
                    for kind, symbol in names):
                continue
            raise ValueError("ROM export table is unavailable for " + dll)
        exports = rom_rva(img, sections, export_rva)
        if exports is None:
            if dll == "ddraw.dll" and all(
                    kind == "name" and symbol == "DirectDrawCreate"
                    for kind, symbol in names):
                continue
            raise ValueError("compressed export table in " + dll)
        base, functions, named, funcs_rva, names_rva, ords_rva = \
            struct.unpack_from("<6I", img, exports + 16)
        exported_names = set()
        for index in range(named):
            name_table = rom_rva(img, sections, names_rva)
            if name_table is None:
                continue
            name_rva = u32(img, name_table + index * 4)
            name_offset = rom_rva(img, sections, name_rva)
            if name_offset is None:
                continue
            end = bytes(img).index(b"\0", name_offset)
            exported_names.add(bytes(img[name_offset:end]).decode("ascii"))
        function_table = rom_rva(img, sections, funcs_rva)
        exported_ordinals = set()
        if function_table is not None:
            exported_ordinals = {base + i for i in range(functions)
                                 if u32(img, function_table + i * 4)}
        for kind, symbol in names:
            if kind == "name" and symbol not in exported_names:
                missing.append(dll + "!" + symbol)
            elif kind == "ordinal" and symbol not in exported_ordinals:
                missing.append(dll + "!ordinal-%d" % symbol)
    if missing:
        raise ValueError("missing ROM imports: " + ", ".join(missing))


def patch_slot(slot, exe):
    if len(slot) != ARM_SLOT_SIZE:
        raise ValueError("ARM slot must be exactly 0x3c00000 bytes")
    img = memoryview(slot)[PREAMBLE:]
    if bytes(img[0x40:0x44]) != ROM_SIG or bytes(img[0x80:0x84]) != b"SUM4":
        raise ValueError("ARM ROM signatures are missing")
    length = u32(img, 0x32)
    if length != u32(img, 0x88) or length != u32(slot, 0):
        raise ValueError("ARM ROM length fields disagree")
    if checksum(img, length) != u32(img, 0x84):
        raise ValueError("source ARM ROM checksum is invalid")
    romhdr = u32(img, 0x44) - PHYSFIRST
    if u32(img, romhdr + 0x0C) - PHYSFIRST != length:
        raise ValueError("ROMHDR physlast does not match image length")
    count = u32(img, romhdr + 0x10)
    toc = romhdr + 0x54
    modules = rom_modules(img, toc, count)
    if "prgupdate.exe" not in modules:
        raise ValueError("PRGUpdate.exe not present in ARM ROM TOC")
    sections, entry, vbase, module_size, imp_rva, imp_size, imports = parse_pe(exe)
    check_imports(img, modules, imports)
    target, old_e32, old_sections = modules["prgupdate.exe"]
    old_load = u32(img, target + 0x1C)
    max_load = 0
    occupied = []
    for name, (entry_off, e32, _) in modules.items():
        load = u32(img, entry_off + 0x1C)
        end = load + u32(img, e32 + 0x14)
        max_load = max(max_load, end)
        if name != "prgupdate.exe":
            occupied.append((load, end))
    for section in sections:
        if section["psize"] > section["vsize"]:
            section["data"] = section["data"][:section["vsize"]]
            section["psize"] = section["vsize"]
    cursor = align(length, 0x1000)
    for section in sections:
        if not section["psize"]:
            section["offset"] = None
            continue
        cursor = align(cursor, 4 if section["flags"] & 0x80000000 else 0x1000)
        section["offset"] = cursor
        cursor += section["psize"]
    cursor = align(cursor, 4)
    o32 = cursor
    cursor += len(sections) * 0x18
    e32 = align(cursor, 4)
    new_length = align(e32 + 0x74, 4)
    if PREAMBLE + new_length > len(slot):
        raise ValueError("replacement module does not fit ARM slot")
    load = old_load
    if any(a < old_load + module_size and b > old_load for a, b in occupied):
        load = align(max_load, 0x1000)
    img[length:new_length] = b"\0" * (new_length - length)
    for section in sections:
        if section["offset"] is not None:
            start = section["offset"]
            img[start:start + section["psize"]] = section["data"]
    for index, section in enumerate(sections):
        data_offset = section["offset"] if section["offset"] is not None else new_length
        flags = section["flags"] & ~0x00F00000
        struct.pack_into("<IIIIII", img, o32 + index * 0x18,
                         section["vsize"], section["rva"], section["psize"],
                         PHYSFIRST + data_offset, vbase + section["rva"], flags)
    struct.pack_into("<HHII", img, e32, len(sections), 0x010F, entry, vbase)
    struct.pack_into("<HH", img, e32 + 0x0C, 5, 0)
    struct.pack_into("<III", img, e32 + 0x10, 0x10000, module_size, 0)
    struct.pack_into("<I", img, e32 + 0x20, u32(img, old_e32 + 0x20))
    struct.pack_into("<II", img, e32 + 0x2C, imp_rva, imp_size)
    struct.pack_into("<H", img, e32 + 0x6C, 9)
    struct.pack_into("<I", img, target + 0x0C, len(exe))
    struct.pack_into("<III", img, target + 0x14,
                     PHYSFIRST + e32, PHYSFIRST + o32, load)
    struct.pack_into("<I", img, romhdr + 0x0C, PHYSFIRST + new_length)
    struct.pack_into("<I", img, 0x32, new_length)
    struct.pack_into("<I", img, 0x88, new_length)
    struct.pack_into("<I", slot, 0, new_length)
    struct.pack_into("<I", img, 0x84, checksum(img, new_length))
    struct.pack_into("<H", img, 0x30,
                     (sum(img[:new_length]) - img[0x30] - img[0x31]) & 0xFFFF)
    img.release()


def sh4_checksum(img, length):
    """Return the QY7 SUM4 value, excluding the two checksum fields."""
    total = 0
    for offset in range(0, length & ~3, 4):
        if 0x30 <= offset < 0x36 or 0x84 <= offset < 0x8C:
            continue
        total += u32(img, offset)
    return total & 0xFFFFFFFF


def find_sh4_slots(image_path):
    """Locate the two QY7 NB images by ECEC and verify their expected slots."""
    found = []
    with open(image_path, "rb") as source:
        with mmap.mmap(source.fileno(), 0, access=mmap.ACCESS_READ) as image:
            offset = image.find(ROM_SIG)
            while offset >= 0:
                slot = offset - NB_HEADER_SIZE - 0x40
                if slot >= 0 and offset + 0x10 <= len(image):
                    xip = offset - 0x40
                    if (u32(image, xip + 0x44) >> 24) == 0x88:
                        romhdr = u32(image, xip + 0x44) - PHYSFIRST + xip
                        if romhdr + 0x54 <= len(image):
                            physfirst = u32(image, romhdr + 0x08)
                            physlast = u32(image, romhdr + 0x0C)
                            modules = u32(image, romhdr + 0x10)
                            cpu = u16(image, romhdr + 0x44)
                            if (physfirst == PHYSFIRST and physlast > physfirst
                                    and 0 < modules < 4096 and cpu == 0x01A6):
                                found.append(slot)
                offset = image.find(ROM_SIG, offset + 1)
    if tuple(found) != SH4_BANK_SLOTS:
        raise ValueError("QY7 SH-4 slots differ from expected offsets: "
                         + ", ".join(hex(value) for value in found))
    return found


def patch_sh4_slot(slot, exe):
    """Replace PRGUpdate.exe in one complete QY7 NB bank."""
    if len(slot) != SH4_BANK_SIZE:
        raise ValueError("SH-4 bank must be exactly 0xa00000 bytes")
    img = memoryview(slot)[NB_HEADER_SIZE:]
    if bytes(img[0x40:0x44]) != ROM_SIG or bytes(img[0x80:0x84]) != b"SUM4":
        raise ValueError("SH-4 ROM signatures are missing")
    length = u32(img, 0x32)
    if length != u32(img, 0x88) or length != u32(slot, 0):
        raise ValueError("SH-4 ROM length fields disagree")
    if sh4_checksum(img, length) != u32(img, 0x84):
        raise ValueError("source SH-4 ROM checksum is invalid")
    romhdr = u32(img, 0x44) - PHYSFIRST
    if u32(img, romhdr + 0x0C) - PHYSFIRST != length:
        raise ValueError("SH-4 ROMHDR physlast does not match image length")
    if u16(img, romhdr + 0x44) != 0x01A6:
        raise ValueError("QY7 ROMHDR is not SH-4")

    count = u32(img, romhdr + 0x10)
    toc = romhdr + 0x54
    target = None
    modules = []
    max_load = 0
    rom = img.tobytes()
    for index in range(count):
        entry = toc + index * 0x20
        name_offset = u32(img, entry + 0x10) - PHYSFIRST
        end = rom.index(b"\0", name_offset)
        name = rom[name_offset:end].decode("latin1").lower()
        e32 = u32(img, entry + 0x14) - PHYSFIRST
        load = u32(img, entry + 0x1C)
        end_load = load + u32(img, e32 + 0x14)
        max_load = max(max_load, end_load)
        if name == "prgupdate.exe":
            target = (entry, e32, index, load)
        else:
            modules.append((load, end_load))
    if target is None:
        raise ValueError("PRGUpdate.exe not present in SH-4 ROM TOC")

    pe = u32(exe, 0x3C)
    if u16(exe, pe + 4) != 0x01A6:
        raise ValueError("SH-4 slot requires a Windows CE SH-4 executable")
    sections, entry, vbase, module_size, imp_rva, imp_size, _ = parse_pe(exe)
    old_entry, old_e32, module_index, old_load = target
    for section in sections:
        if section["psize"] > section["vsize"]:
            section["data"] = section["data"][:section["vsize"]]
            section["psize"] = section["vsize"]

    cursor = align(length, 0x10)
    for section in sections:
        if section["psize"]:
            section["offset"] = cursor
            cursor = align(cursor + section["psize"], 4)
        else:
            section["offset"] = None
    o32 = align(cursor, 4)
    cursor = o32 + len(sections) * 0x18
    e32 = align(cursor, 4)
    cursor = e32 + 0x74
    load = align(cursor, 0x100)
    load_size = align(max(section["vsize"] for section in sections), 0x1000)
    new_length = align(load + load_size, 4)
    if NB_HEADER_SIZE + new_length > len(slot):
        raise ValueError("replacement module does not fit SH-4 bank")

    load_va = old_load
    if any(start < old_load + module_size and finish > old_load
           for start, finish in modules):
        load_va = align(max_load, 0x1000)
    img[length:new_length] = b"\0" * (new_length - length)
    for section in sections:
        if section["offset"] is not None:
            start = section["offset"]
            img[start:start + section["psize"]] = section["data"]
    for index, section in enumerate(sections):
        data_offset = section["offset"] if section["offset"] is not None else new_length
        flags = section["flags"] & ~0x00F00000
        struct.pack_into("<IIIIII", img, o32 + index * 0x18,
                         section["vsize"], section["rva"], section["psize"],
                         PHYSFIRST + data_offset, vbase + section["rva"], flags)
    e32_data = bytearray(0x74)
    struct.pack_into("<HHII", e32_data, 0, len(sections), 0x010F, entry, vbase)
    struct.pack_into("<HH", e32_data, 0x0C, 5, 0)
    struct.pack_into("<III", e32_data, 0x10, 0x10000, module_size, 0)
    struct.pack_into("<I", e32_data, 0x20, u32(img, old_e32 + 0x20))
    struct.pack_into("<II", e32_data, 0x2C, imp_rva, imp_size)
    struct.pack_into("<H", e32_data, 0x6C, 9)
    img[e32:e32 + 0x74] = e32_data
    struct.pack_into("<I", img, old_entry + 0x0C, len(exe))
    struct.pack_into("<III", img, old_entry + 0x14,
                     PHYSFIRST + e32, PHYSFIRST + o32, load_va)
    struct.pack_into("<I", img, romhdr + 0x0C, PHYSFIRST + new_length)
    struct.pack_into("<I", img, 0x32, new_length)
    struct.pack_into("<I", img, 0x88, new_length)
    struct.pack_into("<I", slot, 0, new_length)
    struct.pack_into("<I", img, 0x84, sh4_checksum(img, new_length))
    img.release()
    print("  SH-4 bank module #%d: length 0x%x -> 0x%x, SUM4=0x%08x"
          % (module_index, length, new_length, u32(slot, NB_HEADER_SIZE + 0x84)))


def write_patched_image(base_path, output_path, patches):
    patches.sort(key=lambda item: item[0])
    with open(base_path, "rb") as source, open(output_path, "wb") as output:
        position = 0
        for offset, data in patches:
            if offset < position:
                raise ValueError("image replacement ranges overlap")
            source.seek(position)
            remaining = offset - position
            while remaining:
                block = source.read(min(1024 * 1024, remaining))
                if not block:
                    raise ValueError("base image ended before replacement slot")
                output.write(block)
                remaining -= len(block)
            output.write(data)
            position = offset + len(data)
        source.seek(position)
        while True:
            block = source.read(1024 * 1024)
            if not block:
                break
            output.write(block)


def main():
    args = sys.argv[1:]
    sh4_exe_path = None
    if len(args) == 5 and args[3] == "--sh4-exe":
        base_path, exe_path, output_path, _, sh4_exe_path = args
    elif len(args) == 3:
        base_path, exe_path, output_path = args
    else:
        raise SystemExit("usage: make-card-image.py BASE.img ARM.exe OUTPUT.img "
                         "[--sh4-exe SH4.exe]")
    image_size = __import__("os").path.getsize(base_path)
    with open(exe_path, "rb") as source:
        exe = source.read()
    if image_size < ARM_OFFSET + ARM_SLOT_SIZE:
        raise ValueError("base image is too short for QY8XXX ARM slot")
    patches = []
    with open(base_path, "rb") as source:
        source.seek(ARM_OFFSET)
        arm_slot = bytearray(source.read(ARM_SLOT_SIZE))
    patch_slot(arm_slot, exe)
    patches.append((ARM_OFFSET, arm_slot))
    if sh4_exe_path:
        with open(sh4_exe_path, "rb") as source:
            sh4_exe = source.read()
        for offset in find_sh4_slots(base_path):
            with open(base_path, "rb") as source:
                source.seek(offset)
                sh4_slot = bytearray(source.read(SH4_BANK_SIZE))
            patch_sh4_slot(sh4_slot, sh4_exe)
            patches.append((offset, sh4_slot))
    write_patched_image(base_path, output_path, patches)
    for name, path in (("base", base_path), ("image", output_path)):
        digest = hashlib.sha256()
        with open(path, "rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(block)
        print(name + " sha256:", digest.hexdigest())


if __name__ == "__main__":
    try:
        main()
    except (ValueError, IndexError, struct.error) as error:
        raise SystemExit("make-card-image: " + str(error))
