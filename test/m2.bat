make clean
del .\data\ehcmodule.elf
copy /y ..\ehcmodule\bin\ehcmodule.elf .\data
del test.dol
del usb2_test.dol
make 
ren test.dol usb2_test.dol