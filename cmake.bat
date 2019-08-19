@echo off
rmdir phoenix_vs2017 /s /q
mkdir phoenix_vs2017
cd phoenix_vs2017
cmake.exe -DCMAKE_TOOLCHAIN_FILE=../vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 15 2017" -A x64 ..
cd..

rmdir phoenix_vs2019 /s /q
mkdir phoenix_vs2019
cd phoenix_vs2019
cmake.exe -DCMAKE_TOOLCHAIN_FILE=../vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 16 2019" -A x64 ..
cd..

pause