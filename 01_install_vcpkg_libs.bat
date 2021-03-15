rmdir vcpkg /s /q
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
rem prevent issue with GLM:  https://github.com/microsoft/vcpkg/commit/307ac18b08e4125d3c521a86b7d7b317629be277
git checkout f9763b717eba12be38ea750406fe2a22a7ff00f6
powershell "dir *.ps* -recurse | Set-AuthenticodeSignature -Certificate (dir cert:\currentuser\my -code)"
call bootstrap-vcpkg.bat
rem vcpkg integrate install
vcpkg install --recurse freetype assimp ffmpeg[opencl,nonfree,nvcodec,x264,gpl,version3] stb glm glad imgui glfw3 exprtk --triplet x64-windows-static
cd..