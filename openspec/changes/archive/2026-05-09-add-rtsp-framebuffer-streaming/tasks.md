## 1. Configuration and Lifecycle

- [x] 1.1 Define hardcoded standard RTSP values in the streaming subsystem, including port, path, resolution policy, FPS, bitrate, and codec preference.
- [x] 1.2 Wire streamer lifecycle into `DemoKernel` so it initializes when the no-value `enableStreaming` argument is present.
- [x] 1.3 Ensure streaming does not initialize by default when `enableStreaming` is not present.
- [x] 1.4 Ensure streaming shutdown is called from the normal engine close path.
- [x] 1.5 Add an ImGui Engine Config checkbox that enables or disables RTSP streaming at runtime.

## 2. Streaming Subsystem

- [x] 2.1 Create a `FramebufferStreamer` module with init, frame submission, status, and shutdown methods.
- [x] 2.2 Implement FFMPEG encoder/muxer setup for an RTSP endpoint compatible with VLC.
- [x] 2.3 Handle missing encoder, invalid stream configuration, and port bind failures with clear logs and non-fatal startup behavior.
- [x] 2.4 Add bounded frame buffering so encoding/network delays do not block the render loop indefinitely.
- [x] 2.5 Implement single-client connection handling and client disconnect cleanup without stopping the engine.
- [x] 2.6 Ensure a second client attempt does not disrupt the active stream or render loop.

## 3. Framebuffer Capture

- [x] 3.1 Add final framebuffer capture after the engine render output has been composed and before ImGui/debug overlays are rendered.
- [x] 3.2 Convert captured pixels into the encoder-required orientation and pixel format.
- [x] 3.3 Match stream dimensions to the current window framebuffer size and reconfigure encoding after resize when needed.
- [x] 3.4 Throttle capture to the hardcoded stream FPS independently from render FPS.
- [x] 3.5 Avoid capturing frames when streaming failed to initialize or has no active output path.

## 4. Build Integration

- [x] 4.1 Verify the current vcpkg FFMPEG package exposes the libraries needed for encoding, scaling, muxing, and RTSP output.
- [x] 4.2 Update CMake link/include settings if additional FFMPEG components are required.
- [x] 4.3 Keep standalone builds working when RTSP streaming is compiled in but inactive by default.

## 5. Validation

- [x] 5.1 Build the project with the existing CMake preset.
- [ ] 5.2 Run Phoenix without `enableStreaming` and verify no RTSP stream starts by default.
- [ ] 5.3 Run Phoenix with `enableStreaming` and verify the hardcoded RTSP URL opens in VLC.
- [ ] 5.4 Verify the Engine Config checkbox starts and stops RTSP streaming at runtime.
- [ ] 5.5 Verify behavior when port 554 is already in use.
- [ ] 5.6 Verify local debug ImGui overlays are not visible in the RTSP stream.
- [ ] 5.7 Verify the RTSP stream resolution matches the window and updates after resize.
- [ ] 5.8 Verify a second client attempt does not disrupt the first VLC client or the engine.
- [ ] 5.9 Verify shutdown while a client is connected does not crash or hang.
- [ ] 5.10 Measure render loop impact and confirm overload behavior drops frames instead of freezing the engine.
