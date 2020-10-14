rmdir vcpkg /s /q
REM git clone https://github.com/Spontz/vcpkg.git
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
git checkout 83f88c7a629a8c681fa0f68c207377a9146dd7e5
call bootstrap-vcpkg.bat
vcpkg install --recurse freetype assimp ffmpeg[opencl,nonfree,nvcodec,x264,gpl,version3] stb glm glad imgui glfw3 exprtk --triplet x64-windows-static
cd..