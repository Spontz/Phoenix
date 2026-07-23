cd vcpkg
vcpkg update
rem powershell "dir *.ps* -recurse | Set-AuthenticodeSignature -Certificate (dir cert:\currentuser\my -code)"
call bootstrap-vcpkg.bat
vcpkg install --recurse assimp ffmpeg[opencl,nonfree,nvcodec,x264,gpl,version3] miniaudio kissfft stb glm glad imgui[opengl3-binding,glfw-binding] exprtk uwebsockets libdatachannel[srtp,ws] --triplet x64-windows --keep-going --clean-after-build
cd..
