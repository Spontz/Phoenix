@echo off

cd glfw-3.2.1

echo ==== Spontz Rules, and you Suck!
echo ==== GLFW Project generator ==== VS15(2017) Win32
rmdir build_32 /s /q
mkdir build_32
cd build_32
cmake.exe -G "Visual Studio 15 2017" ..
cd..

echo ==== Spontz still Rules, and you still Suck! ... but in 64 bits
echo ==== GLFW Project generator ==== VS15(2017) x64
rmdir build_64 /s /q
mkdir build_64
cd build_64
cmake.exe -G "Visual Studio 15 2017 Win64" ..
cd..
pause