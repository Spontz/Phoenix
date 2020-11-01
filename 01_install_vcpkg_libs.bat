rmdir vcpkg /s /q
REM git clone https://github.com/Spontz/vcpkg.git
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
git checkout c34c4189ebbf0795e1f8e2de562c527a854c8db4
call bootstrap-vcpkg.bat
vcpkg install --recurse freetype assimp ffmpeg[opencl,nonfree,nvcodec,x264,gpl,version3] stb glm glad imgui glfw3 exprtk --triplet x64-windows-static
cd..