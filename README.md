# Engine Usage keys
## Control keys
* Exit Demo : ESC
* Take Screenshot: PRINTSCREEN -> Not implemented yet
* Show time on log file: ENTER
* Show time on screen: T (only works if the engine is in debug mode)
* Show FPS's on screen: U (only works if the engine is in debug mode)
* Show FBO's content on screen: F (only works if the engine is in debug mode)
* Change FBO's Attachment content on screen: G (only works if the engine is in debug mode)

## Playback control
* PLAY / PAUSE: F1
* REWIND: F2
* FASTFORWARD: F3
* RESTART: F4

## Camera keys
* Capture camera position: SPACE
* Move camera forward: UP
* Move camera backwards: DOWN
* Move camera left: LEFT
* Move camera right: RIGHT
* Change Camera Speed: RIGHT ALT
* Reset camera position: R

# Engine formulas and variables
## Formulas
The formulas that can be used in the engine, can be like this:

`string tx:=0;ty:=-1.75;tz:=0;`

Or, if using formulas in shaders, like this:

`uniform float scale v1:=1+0.25*beat;`

`uniform vec3 color v1:=1.0;v2:=1+5.0*beat;v3:=1.0;`



## Considerations when writing formulas:

* No spaces should be placed into a formula, even between the variables.
* Equal (=) is written like this: `:=`, for example: `uniform float scale v1:=10*beat;`
* At the end of any formula, we should write a semicolon `;`, for example: `uniform float scale v1:=10*beat;`
* If you want to write multiple formulas, split them also with semicolon, for example: `uniform float rotation v1:=90*sin(t);v2:=0;v3:=180*cos(t);`
* A complete description of the formulas you can make can be found here: https://archive.codeplex.com/?p=fastmathparser

## Variables

For shaders, the variables that are sent to the shader are named: `v1`,`v2`,`v3` and `v4`
The other variables that are "built in", and can be used anytime are:

* t: Time of the section in seconds
* tend: Section time end
* tdemo: Time of the demo in seconds
* beat: Beat detection of the music (from 0.0 to 1.0)
* fps: FPS of the demo

Camera data:

* cam_pos[X/Y/Z]: Position of the camera (X/Y/Z)
* cam_up[X/Y/Z]: Up vector of the camera (X/Y/Z)
* cam_yaw: Yaw of the camera
* cam_pitch: Pitch of the camera
* cam_zoom: Zoom value of the camera

Lights data:

N = Light number (form 0 to 3)
* lightN_pos[X/Y/Z]: Light Position (X/Y/Z)
* lightN_dir[X/Y/Z]: Light Direction (X/Y/Z)
* lightN_colAmb[R/G/B]: Light Ambient Color (R/G/B)
* lightN_colDif[R/G/B]: Light Diffuse Color (R/G/B)
* lightN_colSpc[R/G/B]: Light Specular Color (R/G/B)

Viewport data:

* vpWidth: Viewport width of the window (in pixels)
* vpHeight: Viewport height of the window (in pixels)
* aspectRatio: aspectRatio of the window

Fbo data:

* fboXWidth: Width in pixels of the fbo X ('X' goes from 0 to 24)
* fboXHeight: Height in pixels of the fbo X ('X' goes from 0 to 24)

## Writing formulas for shaders
### Using simple variables (float, vec2, vec3 and vec4)
If you want to pass some variables to a shader via `float`,`vec2`,`vec3`or `vec4`, the format should be: `uniform float <variable_name> v1:=<value1>;`, or `uniform vec2 <variable_name> v1:=<value1>;v2:=<value2>;`, for example:

`uniform vec3 fill_color v1:=1;v2:=0;v3:=0;`

This will send a variable named `fill_color` with the value of (1,0,0) (red).

`uniform float scale v1:=1+0.1*beat;`

This will send a float variable named `scale` with a variable value, depending on the `beat` of our demo.

### Using texture variables (sampler2D)
In case we want to use a `sampler2D` variable (texture), the format should be: `uniform sampler2D <variable_name> <texture_unit> <texture_filename_path>`, for example:

`uniform sampler2D texture_1 0 /pool/textures/my_texture.jpg`

This will send a texture placed in `/pool/textures/my_texture.jpg` to the variable named `texture_1`, the `0` is required because it indicates the texture unit to use.

`uniform sampler2D texture_1 0 /pool/textures/my_texture.jpg`

`uniform sampler2D texture_2 1 /pool/textures/my_texture_2.jpg`

In this case we will send 2 textures, one in texture unit 0 and the other one in the texture unit 1.

`uniform sampler2D texture_1 0 fbo2`

In this case we are sending to the shader a texture named `texture_1` with the content of the `fbo` number 2, everything loaded in texture unit 0.
