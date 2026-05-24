# RTSP Streaming

Phoenix can expose the final OpenGL framebuffer as an RTSP video stream for local monitoring in VLC or another RTSP client.

## Activation

Streaming is disabled by default. Enable it at startup with the no-value argument:

```bat
Phoenix.exe enableStreaming
```

The flag is independent from slave mode. A Phoenix instance can run with streaming enabled or disabled regardless of its network control mode.

Streaming can also be toggled while Phoenix is running:

1. Open the ImGui `Config` panel.
2. Use the `Enable RTSP streaming` checkbox.

Disabling the checkbox closes the active RTSP client, listener, encoder, and queued frames. Enabling it again recreates the listener so VLC can reconnect without restarting Phoenix.

## Endpoint

The stream listens on the hardcoded standard RTSP port:

```text
rtsp://localhost:554
```

Only one client is supported in the first implementation. A second client attempt must not disrupt the active stream or the engine.

Port `554` may require elevated privileges or may already be used by another service. If binding fails, Phoenix logs the streaming startup failure and continues running normally.

## Video

The stream contains the final demo framebuffer before ImGui debug overlays are rendered. Local debug UI remains visible in the Phoenix window, but it is not included in the RTSP output.

The encoded resolution follows the current window framebuffer size. If the window is resized while streaming is active, subsequent frames use the new window size. Dimensions are adjusted only as needed for encoder-compatible even values.

Current hardcoded stream values:

- RTSP port: `554`
- RTSP path: `/`
- FPS cap: `30`
- Bitrate: `4000000`
- Codec preference: H.264, preferring `libx264` when available

## Runtime Behavior

Framebuffer capture is throttled independently from render FPS. The frame queue is bounded, so if encoding or network output falls behind, Phoenix drops queued frames instead of blocking the render loop indefinitely.

If the client disconnects, Phoenix continues rendering. Streaming can be stopped and started again from Engine Config, and clients should be able to reconnect to the same URL.
