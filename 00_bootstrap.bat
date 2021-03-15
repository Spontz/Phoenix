call 01_install_vcpkg_libs.bat
powershell "'.\02_install_third_party_libs.ps1' | Set-AuthenticodeSignature -Certificate (dir cert:\currentuser\my -code)"
Powershell.exe -executionpolicy remotesigned -File .\02_install_third_party_libs.ps1
call 03_run_cmake.bat
pause