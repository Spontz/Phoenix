git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
call bootstrap-vcpkg.bat
vcpkg install freetype assimp ffmpeg stb glm glad glfw3 --triplet x64-windows-static
pause