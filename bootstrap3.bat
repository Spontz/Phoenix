@echo off
rmdir phoenix_vs2017 /s /q
mkdir phoenix_vs2017
cd phoenix_vs2017
rem cmake.exe -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 15 2017" -A x64 ..
cmake.exe -G "Visual Studio 15 2017" -A x64 ..
cd..

pause

rmdir phoenix_vs2019 /s /q
mkdir phoenix_vs2019
cd phoenix_vs2019
rem cmake.exe -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 16 2019" -A x64 ..
cmake.exe -G "Visual Studio 16 2019" -A x64 ..
cd..

pause