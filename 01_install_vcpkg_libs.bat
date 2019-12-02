rmdir vcpkg /s /q
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
git pull
git checkout e99a3f913202bc3da77ccf4b65391e245079eb05
call bootstrap-vcpkg.bat
vcpkg install freetype assimp ffmpeg stb glm glad glfw3 exprtk --triplet x64-windows-static
cd..