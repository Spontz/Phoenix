rmdir vcpkg /s /q
REM git clone https://github.com/Spontz/vcpkg.git
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
rem checkout from Dec18th
git checkout 66bac12e4e54cc858bdf7f93c5786010b6812b13
call bootstrap-vcpkg.bat
vcpkg install --recurse freetype assimp ffmpeg[opencl,nonfree,nvcodec,x264,gpl,version3] stb glm glad imgui[docking-experimental] glfw3 exprtk --triplet x64-windows-static
cd..