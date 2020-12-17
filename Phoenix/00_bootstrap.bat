call 01_install_vcpkg_libs.bat
Powershell.exe -executionpolicy remotesigned -File .\02_install_third_party_libs.ps1
call 03_run_cmake.bat
pause