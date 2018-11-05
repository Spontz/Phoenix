@echo off
echo,
echo VS15(2017) x86 (Win32) -----------------------------------
rmdir build_vs15_x86 /s /q
mkdir build_vs15_x86
cd build_vs15_x86
cmake.exe -G "Visual Studio 15 2017" ..
cd..
echo,
echo VS15(2017) x64 (Windows 64 bit) --------------------------
rmdir build_vs15_x64 /s /q
mkdir build_vs15_x64
cd build_vs15_x64
cmake.exe -G "Visual Studio 15 2017 Win64" ..
cd..
pause