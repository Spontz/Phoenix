set project=phoenix

rmdir %project%_vs2022 /s /q
mkdir %project%_vs2022
cd %project%_vs2022
cmake.exe .. -DBUILD_SHARED_LIBS=ON -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 17 2022"
cd..

pause