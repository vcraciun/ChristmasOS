::prepare files
attrib -s -h adu.exe                                                   > nul
::copy adu.exe ..\IMAGES\BOCHS-QEMU\adu.exe                              > nul
copy mbr.bin ..\IMAGES\BOCHS-QEMU\mbr.bin                              > nul
copy ..\Release\binary_test.bin ..\IMAGES\BOCHS-QEMU\binary_test.bin   > nul

::update BOCHS/QEMU image                                              
cd ..\IMAGES\BOCHS-QEMU                                                       
echo writting MBR to IMAGES\BOCHS-QEMU\bootmanager.img
adu.exe /wi /im=bootmanager.img /ss=0  /df=mbr.bin                     > nul
echo writting Kernel to IMAGES\BOCHS-QEMU\Bootmanager.img
adu.exe /wi /im=bootmanager.img /ss=1  /df=binary_test.bin             > nul
del mbr.bin                                                            > nul
del binary_test.bin                                                    > nul

cd ..\..
