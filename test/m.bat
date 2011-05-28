del test.elf
del test.dol
del .\build\ehcmodule.elf.o
del .\data\ehcmodule.elf
copy /y ..\ehcmodule\bin\ehcmodule.elf .\data
make 
wiiload test.dol
