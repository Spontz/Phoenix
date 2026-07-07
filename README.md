# Phoenix :: Spontz demo Engine
Welcome to the infamous Phoenix :: Spontz demoengine. Be aware that each line of this code has been written with tons of suffering and [untamed indomitus nighly] pain. Please take into consideration that this could (would) extent to you if you decide to take a look.

This demo engine is an old piece of software that has undergone several heavy updates since its original C version. We have released about twenty demos with it. Main efforts come from:

* Kolian: The main developer of the original 'Inferno/Dragon' Spontz demoengine.
* Spöntz members: isaac2, Khrome, merlucin, madgoblin, shotgan, xphere.

This engine would not exist without inspiration obtained from the following sources. Thanks!

* Yan Chernikov / aka 'The Cherno': https://github.com/TheCherno
* Etay Meiri / OGL Dev site: https://ogldev.org/
* jmorton06 / Lumos engine: https://github.com/jmorton06/Lumos

Phoenix uses the following libs:

* GLFW: OpenGL multiplatform library - https://github.com/glfw/glfw
* GLAD: OpenGL Extension manager - https://glad.dav1d.de
* GLM: OpenGL mathematics - https://github.com/g-truc/glm
* MiniAudio: Audio playback - https://miniaud.io
* KissFFT: FFT Analysis - https://github.com/mborgerding/kissfft
* Assimp: 3d asset loading - https://github.com/assimp/assimp
* STB: bitmap loading - https://github.com/nothings/stb
* Exprtk: mathematical expression evaluator - https://github.com/ArashPartow/exprtk
* FFMpeg: Video decoder - https://github.com/FFmpeg/FFmpeg
* Dear ImGUI: graphical user interface - https://github.com/ocornut/imgui
* dyad.c: networking - https://github.com/rxi/dyad

## Instructions (Windows)

Please run 00_bootstrap.bat, this will download all required dependecies and create the vs2022 project, then compile the project usign Visual Studio. Feel free to contact any of us if you need further assistance.

## Debug launch with Cacablu

When launching the Debug build outside Visual Studio, always use the same working directory configured by CMake and pass the matching data folder explicitly:

```powershell
Start-Process `
  -FilePath ".\phoenix_vs2026\Launcher\Debug\Phoenix.exe" `
  -WorkingDirectory ".\Launcher_WD" `
  -ArgumentList "-datafolder `".\Launcher_WD\data`""
```

`Launcher_WD\data\config\control.spo` must contain `slave 1` for the editor API to listen on `127.0.0.1:29100`.

# Screenshots

<p align="center">
  <img src="img/engine.png">
  <img src="img/engine_glow.png">
  <img src="img/engine_raymatching.png">
  <img src="img/engine_envMap.png">
  <img src="img/engine_anim.png">
</p>


<p align="center">
  <img width="400" height="400" src="/img/spontzLogo.jpg">
</p>
