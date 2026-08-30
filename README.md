# Leaf SD Tools
<img src="https://github.com/developerfromjokela/leafsdtools/blob/master/leafsdtool.png" height="400">
<img src="https://github.com/developerfromjokela/leafsdtools/blob/master/leafsdtool_qy8xxx.png" height="400">
Modify, backup and unlock your Head Unit with all-in-one utility.
No hardware modifications are required.

## Current working functionality
- [x] Backup NAND
- [x] Write NAND
- [x] Lock/Unlock SD cards using slot A
- [x] Read&Write SRAM and VFlash (User Data)
- [x] Retrieve SD Card pin
- [x] Read&Write Product INF (For now only on QY7XXX)
- [x] Disable IMMO (For now only on QY7XXX)

## Using the program
You'll need a full-size SD card (or micro-SD with an adapter), with at least (10 GB for versions older than 110, starting from 110, 600 MB is enough) of capacity.
Download latest disk image release of LEAF SD tools and write it to the SD with tools like:
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

> With version 104, you can also exit Software Update mode by pressing EXIT UPDATE button in main menu

### GUIDE FOR QY8XXX
1. Tilt the screen and remove SD. Insert SD with Leaf SD Tools
2. Turn off and turn back on. The program will boot in a moment.

TO EXIT THE PROGRAM:
1. Press Exit Update
2. Confirm the exit
3. Once green screen appears, remove the SD and insert your stock card in. Turn off and back on, you'll boot back to the OS.

**NOTE FOR QY7XXX ONLY! Make sure your stock SD is locked before exiting update mode! Head Unit won't boot back if SD is unlocked.**
