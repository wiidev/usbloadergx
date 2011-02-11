make
if not %errorlevel% == 0 goto end
@echo off
cd bin
call convert.bat
cd ..

:end
