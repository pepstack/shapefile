@echo off

echo Current directory: %CD%

echo clean msvc dirs...
echo.

rmdir /s /q "%~dp0msvc\.vs" 2>nul
rmdir /s /q "%~dp0msvc\libshapefile\build" 2>nul
rmdir /s /q "%~dp0msvc\libshapefile\output" 2>nul
rmdir /s /q "%~dp0msvc\libshapefile\.vs" 2>nul
rmdir /s /q "%~dp0build" 2>nul

echo clean all success.
pause
