set project=phoenix

rmdir %project%_vs2026 /s /q
mkdir %project%_vs2026
cd %project%_vs2026
cmake.exe .. -DBUILD_SHARED_LIBS=ON -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 18 2026"
cd..

pause