cd vcpkg
git pull
.\bootstrap-vcpkg.bat
.\vcpkg install --recurse freetype assimp ffmpeg[opencl,nonfree,nvcodec,x264,gpl,version3] stb glm glad imgui[docking-experimental] glfw3 exprtk --triplet x64-windows-static
.\vcpkg upgrade --no-dry-run
cd..
pause