cd vcpkg
git pull
.\bootstrap-vcpkg.bat
.\vcpkg install freetype assimp ffmpeg stb glm glad imgui glfw3 exprtk --triplet x64-windows-static
.\vcpkg upgrade --no-dry-run
cd..
pause