rmdir vcpkg /s /q
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
git pull
call bootstrap-vcpkg.bat
vcpkg install freetype assimp ffmpeg stb glm glad glfw3 exprtk --triplet x64-windows
cd..