rmdir vcpkg /s /q
git clone https://github.com/Spontz/vcpkg.git
cd vcpkg
git pull
call bootstrap-vcpkg.bat
vcpkg install freetype assimp ffmpeg stb glm glad imgui glfw3 exprtk --triplet x64-windows-static
cd..