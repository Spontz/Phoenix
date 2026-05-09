## Why

Phoenix's rendered output currently needs a local display to be inspected. Exposing the OpenGL framebuffer through RTSP allows operators to monitor a running instance from tools such as VLC using the standard RTSP port when streaming is explicitly requested.

## What Changes

- Add optional RTSP streaming of the final OpenGL framebuffer when Phoenix is launched with the `enableStreaming` argument.
- Capture the rendered output after the section pipeline has produced the final frame, before or around the window buffer swap.
- Encode frames into a video stream suitable for VLC playback over RTSP.
- Listen on the standard RTSP port and expose a stable URL for clients.
- Use hardcoded standard streaming values for port, path, FPS, bitrate, and codec; activation is controlled by a no-value CLI argument.
- Match the stream resolution to the current window resolution, including after window resize.
- Log stream startup, connection, runtime errors, and shutdown clearly.
- Ensure normal rendering continues unchanged when streaming is not explicitly enabled.

## Capabilities

### New Capabilities
- `rtsp-framebuffer-streaming`: Covers activation, capture, encoding, serving, and shutdown behavior for streaming the engine framebuffer over RTSP.

### Modified Capabilities

None.

## Impact

- Affected runtime code: `DemoKernel`, `Window`, and the main render loop timing around framebuffer presentation.
- New engine subsystem likely needed for framebuffer capture and RTSP output.
- FFMPEG integration may expand from decode-only video use to encode/mux/network output.
- Build configuration may need additional FFMPEG libraries or feature checks, depending on the currently linked vcpkg FFMPEG package.
- Network behavior: binds the standard RTSP port, so failures due to port conflicts or permissions must be handled gracefully.
- Operational behavior: disabled by default and enabled only by the `enableStreaming` argument.
