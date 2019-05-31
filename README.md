# Engine Usage keys
## Control keys
* Exit Demo : ESC
* Take Screenshot: PRINTSCREEN -> Not implemented yet
* Show time on log file: ENTER
* Show time on screen: T (only works if the engine is in debug mode)
* Show FPS's on screen: U (only works if the engine is in debug mode)
* Show FBO's content on screen: F (only works if the engine is in debug mode)

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
[source]
----
string tx:=0;ty:=-1.75;tz:=0;
----
Or, if using shaders, like this:
[source]
----
string float scale v1:=1+0.25*beat;
string vec3 color v1:=1.0;v2:=1+5.0*beat;v3:=1.0;
----

## Considerations when writing formulas:

* No spaces should be placed into a formula, even between the variables.
* Equal (=) is written like this: `:=`, for example: `string float scale v1:=10*beat;`
* At the end of any formula, we should write a semicolon `;`, for example: `string float scale v1:=10*beat;`
* If you want to write multiple formulas, split them also with semicolon, for example: `string float rotation v1:=90*sin(t);v2:=0;v3:=180*cos(t);`

## Variables

For shaders, the variables that are sent to the shader are named: `v1`,`v2`,`v3` and `v4`
The other variables that are "built in", and can be used anytime are:

* t: Time of the section in seconds
* tend: Section time end
* tdemo: Time of the demo in seconds
* beat: Beat detection of the music (from 0.0 to 1.0)
* fps: FPS of the demo

Camera data:

* cam_pos_x: Position (x) of the camera
* cam_pos_y: Position (y) of the camera
* cam_pos_z: Position (z) of the camera
* cam_up_x: Up vector (x) of the camera
* cam_up_y: Up vector (y) of the camera
* cam_up_z: Up vector (z) of the camera
* cam_yaw: Yaw of the camera
* cam_pitch: Pitch of the camera
* cam_zoom: Zoom value of the camera

Viewport data:

* vpWidth: Viewport width of the window
* vpHeight: Viewport height of the window
* aspectRatio: aspectRatio of the window

Fbo data:

* fboXWidth: Width of the fbo X (x goes from 0 to 24)
* fboXHeight: Height of the fbo X (x goes from 0 to 24)