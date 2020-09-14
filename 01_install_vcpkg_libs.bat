rmdir vcpkg /s /q
REM git clone https://github.com/Spontz/vcpkg.git
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
git checkout bff4dd9c944b9fbad40ec1a36d7c2365287ce0a6
call bootstrap-vcpkg.bat
vcpkg install freetype assimp ffmpeg stb glm glad imgui glfw3 exprtk --triplet x64-windows-static
cd..