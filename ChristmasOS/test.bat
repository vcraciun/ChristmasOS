@echo off
del *.obj
del *.bin
echo Building CRT
fasm crt.asm -s crt.a
listing.exe crt.a crt.lst -a
echo Building MBR
fasm mbr.asm -s mbr.a
listing.exe mbr.a mbr.lst -a