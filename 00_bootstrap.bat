call 01_install_vcpkg_libs.bat
powershell "'.\02_install_third_party_libs.ps1' | Set-AuthenticodeSignature -Certificate (dir cert:\currentuser\my -code)"
call 02_run_cmake.bat
pause