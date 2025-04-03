# ChristmasOS
An educational Operating System

## Requirements:
- FASM : https://flatassembler.net/
- QEMU 0.9.0
- BOCHS
- PETER-BOCHS debuger
- MSVC
- ADU
- BinDump project to extract the code from the EXE file after building with MSVC


## Dependencies?:
1. Before building the project make sure fasm.exe is in %PATH%
2. Resources folder contains some wallpaper images
3. IMAGES archive contains some already available tools to build the project
   - adu.exe is used to write MBR, kernel and FAT32 volume, to virtual_disk.img
   - qemu-img.exe is used to create fresh disk image files
   - qemu-system-x86_x64.exe is used to emulate the OS
   - PauseBochs.exe and StopBochs.exe are used to stop and pause Bochs emulator
   - vdk.exe is used to map disk image files to partitions, maybe not working any more
   - peter-bochs-debugger20130922.jar is a Bochs emulator controller, requires java
4. you need to install OSFMount to map the fat32.img image file, to a OS partition and maybe change the content

## How to build?
1. Build the project using MSVC, make sure you first build bin_dump
2. After building the EXE, because MSVC does not use standard libraries for this application, dumping the binary with bin_dump, will extract only the code inside the EXE as a payload file
3. during the build of the project, a source file named MBR.asm builds to MBR.bin
4. MBR.bin and the binary dump from ChristmasOS.EXE must be concatenated and written with adu.exe, to virtual_disk.img
   execute: adu.exe /wi /im=virtual_disk.img /ss=0 /df=bootmanager.img (after bootmanager.img contains both mbr.bin and ChristmasOS.bin)
5. fat32.img must be written at 0x20000 (0x100 sectors 0x200 size each) to virtual_disk.img
6. execute: qemu -m 128 -localtime -std-vga -L . virtual_disk.img


## ToDo
1. Add support for a proprietary filesystem, or full FAT32 / NTFS
2. Isolate kernel and user modes
3. Allow 64 bit support
4. Implement some applications
5. Design themes and desktop
6. Add support for Network and GPU
6. Add support for ML//LLM such that the OS will create binary applications on demand, and destroy them after usage


## Screenshot
[!Screenshot](screenshot.png)