#!/usr/bin/env python3
"""Download and verify the pinned upstream LeafSDTools release image."""
import hashlib
import json
import pathlib
import tarfile
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parent.parent
PIN = json.loads((ROOT / "ci/upstream-image.json").read_text())
OWNER = "developerfromjokela"
REPO = "leafsdtools"


def main():
    url = "https://github.com/{}/{}/releases/download/{}/{}".format(
        OWNER, REPO, PIN["tag"], PIN["asset"])
    archive_path = ROOT / "build" / PIN["asset"]
    archive_path.parent.mkdir(parents=True, exist_ok=True)
    request = urllib.request.Request(url, headers={"User-Agent": "LeafSDTools-CI"})
    digest = hashlib.sha256()
    with urllib.request.urlopen(request, timeout=120) as response, \
            archive_path.open("wb") as output:
        while True:
            block = response.read(1024 * 1024)
            if not block:
                break
            output.write(block)
            digest.update(block)
    actual = digest.hexdigest()
    if actual.lower() != PIN["sha256"].lower():
        archive_path.unlink(missing_ok=True)
        raise SystemExit("upstream archive sha256 mismatch: " + actual)
    output_path = ROOT / "build" / "LeafSDTools-base.img"
    try:
        with tarfile.open(archive_path, "r:gz") as archive:
            members = [member for member in archive.getmembers()
                       if member.isfile() and member.name.endswith(".img")]
            if len(members) != 1:
                raise SystemExit("release archive must contain exactly one .img")
            source = archive.extractfile(members[0])
            with output_path.open("wb") as output:
                while True:
                    block = source.read(1024 * 1024)
                    if not block:
                        break
                    output.write(block)
    except (OSError, tarfile.TarError) as error:
        raise SystemExit("cannot extract release image: " + str(error))
    print("verified release archive sha256:", actual)
    print("base image:", output_path)


if __name__ == "__main__":
    main()
