# Leaf SD Tools
<img src="https://github.com/developerfromjokela/leafsdtools/blob/master/leafsdtool.jpeg" height="400">
<img src="https://github.com/developerfromjokela/leafsdtools/blob/master/leafsdtool_qy8.jpeg" height="400">
Modify, backup and unlock your Head Unit with all-in-one utility.
No hardware modifications are required.

## Current working functionality
- [x] Backup NAND
- [x] Write NAND
- [x] Lock/Unlock SD cards using slot A
- [x] Read&Write SRAM and VFlash (User Data)
- [x] Retrieve SD Card pin
- [x] Read&Write Product INF
- [x] Disable IMMO handshake / SECURE screen

## Using the program
You'll need a full-size SD card (or micro-SD with an adapter), with at least (10 GB for versions older than 110, starting from 110, 600 MB is enough) of capacity.
Download latest disk image release of LEAF SD tools and write it to the SD with tools like:
- LeafSDTools Companion Restore function ([github.com/developerfromjokela/leafsdtools_companion](https://github.com/developerfromjokela/leafsdtools_companion))
- balenaEtcher
- DD (`dd if=./LeafSDTools-100-beta.img of=/dev/<device> bs=8M status=progress`)
- Win32DiskImager

## Launching
Launching the program happens from diagnostic menu of Head Unit.
Have your SD card flashed with latest disk image available in releases.

Supported Head Unit models: QY7XXX (CarWings), QY8XXX (NissanConnect)
### GUIDE FOR QY7XXX
1. With the head unit fully booted, enter the map view.
2. Turn off media
3. Press three times "MAP" button, then press media on/off button two times and finally press "MAP" button once again.
4. Open Confirmation/Adjustment menu and scroll to Software Update menu
5. Start the update process. Head unit should reboot.
6. Once prompted a YES/NO dialog, remove SD from MAP Slot (B) and insert your own SD card with flashed image.
7. Turn off the car and start it again.

To exit from Software Update mode, insert stock SD card back and press YES when cancellation option comes available.  

> With version 104 and newer, you can also exit Software Update mode by pressing EXIT UPDATE button in main menu

### GUIDE FOR QY8XXX
1. Tilt the screen and remove SD. Insert SD with Leaf SD Tools
2. If LeafSDTools does not launch automatically, turn ACC power off and turn back on. The program should boot in a moment, 5-10 sec.
3. When you are done, REMEMBER TO EXIT UPDATE! Otherwise you will be stuck in update mode.


TO EXIT THE PROGRAM:
1. Press Exit Update
2. Confirm the exit
3. Once green screen appears, remove the SD and insert your stock card in. Turn off and back on, you'll boot back to the OS.

**NOTE FOR QY7XXX ONLY! Make sure your stock SD is locked before exiting update mode! Head Unit won't boot back if SD is unlocked.**


## Building without eMbedded Visual C++

Both variants are built on Linux with the open CeGCC toolchain from
[palian-tech/cegcc-build](https://github.com/palian-tech/cegcc-build)
(GCC 9.3; `arm-mingw32ce` and `sh4-mingw32ce`, the latter following the
calling convention of the Microsoft SH-4 compiler), published as a Docker
image:

```sh
docker run --rm -v "$PWD":/w -w /w ghcr.io/palian-tech/cegcc:v9.3.0-ce.1 \
  sh -c 'make TARGET=arm check && make TARGET=sh4 check'
# -> build/arm/LeafSDTools.exe (QY8XXX), build/sh4/LeafSDTools.exe (QY7XXX)
```

GitHub Actions runs the same build on every push and pull request and
uploads both executables as artifacts. The sources are compiled unchanged;
what the toolchain lacks is supplied from `compat/`. One difference from the
eVC4 build: structured exception handling (`__try`/`__except`) is compiled
out.

## Building a card image

The card image starts from the upstream `LeafSDTools-111-beta.img` release.
The CI workflow pins and verifies the upstream release archive SHA-256, then
replaces `PRGUpdate.exe` in the QY8XXX ARM updater image and in both mirrored
QY7XXX SH-4 updater banks. Other card contents are preserved. Run the `Build`
workflow with `workflow_dispatch` or push a release tag to build the
compressed `.img.gz` artifact and its `.sha256` file. Ordinary pushes build
only the two executables.

Locally, inside the same image (Python 3 is needed for the image builder):

```sh
make card-image BASE_IMAGE=build/LeafSDTools-base.img
# -> build/LeafSDTools.img(.gz, .sha256)
```

The image builder verifies each PE machine type, locates both 10 MiB QY7XXX
banks from their ROM signatures and places uncompressed module sections on
4 KiB boundaries, as the Windows CE ROM loader requires.
