rmdir phoenix_vs2017 /s /q
mkdir phoenix_vs2017
cd phoenix_vs2017
cmake.exe -DVCPKG_TARGET_TRIPLET=x64-windows-static -G "Visual Studio 15 2017" ..
cd..

rmdir phoenix_vs2019 /s /q
mkdir phoenix_vs2019
cd phoenix_vs2019
cmake.exe -DVCPKG_TARGET_TRIPLET=x64-windows-static -G "Visual Studio 16 2019" ..
cd..

pause