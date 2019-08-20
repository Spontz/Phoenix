call install_vcpkg_libs.bat
Powershell.exe -executionpolicy remotesigned -File .\install_third_party_libs.ps1
call run_cmake.bat
