## Context

Phoenix already links FFMPEG for video decoding and renders its demo timeline through `DemoKernel`, `SectionLayer`, `Window`, and OpenGL FBO helpers. Streaming is an optional runtime monitoring feature and should be independent from slave mode or other networking/control behavior.

The requested workflow is operational: run a Phoenix instance with streaming explicitly enabled and inspect its current visual output from VLC. The stream should therefore be opt-in, robust enough for live monitoring, and isolated from normal rendering.

## Goals / Non-Goals

**Goals:**
- Start an RTSP stream when Phoenix is launched with the no-value `enableStreaming` argument.
- Capture the final OpenGL framebuffer after the render pipeline has produced the frame intended for display.
- Encode frames into a VLC-compatible video stream.
- Serve RTSP on port 554 with a predictable stream URL.
- Keep port, path, FPS, bitrate, and codec values hardcoded to standard defaults.
- Match the stream resolution to the current window resolution, including after resize.
- Shut down cleanly with the engine and avoid changing standalone behavior.
- Log enough information to diagnose missing codecs, port conflicts, capture failures, and client disconnects.

**Non-Goals:**
- Audio streaming.
- Multi-stream output or per-FBO stream selection.
- A full media server with authentication, transcoding profiles, or multi-client management.
- Editor UI for configuring stream settings.
- SPO controls for stream settings in this first iteration.
- Coupling streaming activation to slave mode.

## Decisions

### Add a dedicated framebuffer streaming subsystem

Create a small runtime component owned by `DemoKernel`, for example `FramebufferStreamer`, with lifecycle methods such as `init`, `submitFrame`, and `shutdown`.

Rationale: capture, encoding, network output, and buffering have different failure modes than rendering. Keeping them outside `Window` and `SectionLayer` avoids mixing media output concerns into the core render path.

Alternative considered: place RTSP logic directly in `DemoKernel::Run`. This is simpler initially but would make the main loop harder to reason about and harder to disable or test.

### Use FFMPEG for encoding and RTSP output

Reuse the existing FFMPEG dependency to create an encoder/muxer pipeline. Prefer H.264 if available in the linked FFMPEG build, with a fallback to a clearly logged failure if no suitable encoder exists.

Rationale: the project already depends on FFMPEG headers and libraries, VLC handles H.264 over RTSP well, and adding another streaming stack would increase build and deployment complexity.

Alternative considered: implement RTP/RTSP manually. This is high risk and duplicates mature protocol handling.

### Capture the final default framebuffer without debug overlays

Capture after the section render pass and after `Window::SetFramebuffer()` has restored the default framebuffer, but before ImGui/debug overlays are rendered. The RTSP stream must show the demo output only, even when debug UI is enabled locally.

Rationale: the operator wants to see what the engine is outputting, not a specific internal FBO. Capturing the default framebuffer is the least surprising contract.

Alternative considered: stream one of `m_fboManager`'s color attachments. That is useful for debugging but ambiguous for demos because the final composition may happen across multiple FBOs and sections.

### Support one VLC client in the first implementation

The first implementation only needs to support one RTSP client at a time. If a second client attempts to connect while one client is active, the system should reject it or allow the backend to report that the stream is unavailable, while keeping the engine running.

Rationale: the immediate monitoring workflow is a single operator opening the slave output in VLC. Multi-client serving can be added later if production use needs it.

Alternative considered: build multi-client handling immediately. That increases server state and testing complexity before the single-client capture/encode path is proven.

### Avoid blocking the render loop on network output

Readback and encoding should be buffered so transient client/network stalls do not freeze rendering. A bounded queue should drop older frames when overloaded and log at a throttled rate.

Rationale: slave mode still needs to keep responding to timeline/control events. Monitoring should degrade by dropping frames before it disrupts the engine.

Alternative considered: synchronous encode/write in the render loop. This is easier but risks large frame-time spikes and apparent hangs when VLC disconnects or the network stalls.

### Hardcode standard streaming values for the first iteration

Use fixed initial values in code, including port `554`, the root RTSP URL (`rtsp://localhost:554` for local viewing), a standard codec/profile, and conservative FPS/bitrate defaults. The stream starts only when `DemoKernel::getArguments` sees the no-value `enableStreaming` argument.

Rationale: the first goal is to prove monitoring from VLC with minimum surface area while making activation explicit. Hardcoded stream defaults reduce parser and compatibility work while the capture/encoding/RTSP path is still being established.

Alternative considered: add SPO configuration immediately. That is useful later but expands the first implementation before the streaming path itself is validated.

### Match stream resolution to the window

The streamer should encode frames at the size captured from the window framebuffer. When the window is resized, subsequent captured frames should use the new resolution and the encoder should be recreated if its current output size no longer matches.

Rationale: operators should see the same output shape and resolution as the running Phoenix window instead of a fixed 1280x720 stream that may scale or distort the image.

## Risks / Trade-offs

- Standard RTSP port conflict -> detect bind/listen failure, log it, and continue the engine without streaming.
- FFMPEG build lacks an H.264 encoder -> fail streaming startup with a clear error and continue rendering.
- `glReadPixels` stalls the GPU -> start with a simple implementation, then introduce PBO-based asynchronous readback if performance is unacceptable.
- Frame orientation, pixel format, or odd resize dimensions -> normalize captured data before encoding, keep H.264-compatible even dimensions, and verify in VLC.
- Network backpressure causes frame-time spikes -> use a bounded queue and drop frames instead of blocking.
- Second client connects during active playback -> reject or fail the second connection without affecting the active stream or engine.

## Migration Plan

1. Add hardcoded streaming defaults in the new streaming subsystem.
2. Implement the streaming subsystem so it starts only when `enableStreaming` is passed.
3. Validate local playback with VLC using the documented RTSP URL.
4. If streaming fails at runtime, log and continue without affecting demo execution.
5. Rollback is removing the new subsystem wiring or disabling its slave-mode initialization in code.

## Open Questions

None for the first implementation.
