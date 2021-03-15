set project=phoenix

rmdir %project%_vs2019 /s /q
mkdir %project%_vs2019
cd %project%_vs2019
rem cmake.exe .. -DBUILD_SHARED_LIBS=ON -DVCPKG_TARGET_TRIPLET=x64-windows-static -G "Visual Studio 16 2019"
cmake.exe .. -DVCPKG_TARGET_TRIPLET=x64-windows-static -G "Visual Studio 16 2019"

cd..

pause