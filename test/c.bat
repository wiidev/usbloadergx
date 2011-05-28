rem make clean
copy /y C:\devkitPro\msys\home\Julian\cios_mload\ehcmodule\bin\ehcmodule.elf .\data
del /q .\build\ehcmodule.elf.o
del /q usb.dol
del /q usb.elf
make
copy /y USB.dol H:\apps\usb2\boot.dol