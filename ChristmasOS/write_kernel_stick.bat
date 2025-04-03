@echo off

::prepare files
adu.exe /wd /drive=1 /ss=0  /df=mbr.bin
adu.exe /wd /drive=1 /ss=1  /df=..\Release\binary_test.bin
