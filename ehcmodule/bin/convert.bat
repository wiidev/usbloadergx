del *.c
del *.h
filetochar ehcmodule.elf ehcmodule_5 -h -align 32
del C:\devkitPro\soft\usbloader\source\mload\modules\ehcmodule_5.c
del C:\devkitPro\soft\usbloader\source\mload\modules\ehcmodule_5.h
copy ehcmodule_5.c C:\devkitPro\soft\usbloader\source\mload\modules\
copy ehcmodule_5.h C:\devkitPro\soft\usbloader\source\mload\modules\
touch C:\devkitPro\soft\usbloader\source\mload\modules\ehcmodule_5.c
touch C:\devkitPro\soft\usbloader\source\mload\modules\ehcmodule_5.h




