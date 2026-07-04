## Summary

Replace Phoenix's optional RTSP framebuffer stream with a browser-compatible WebRTC preview stream that Cacablu can render in its Preview panel.

## Motivation

The RTSP stream is useful for VLC, but browsers cannot display it directly without an intermediate backend or transcoder. Cacablu needs to show Phoenix output in the editor preview with minimal latency and without adding a Cacablu backend.

## Scope

- Remove the active RTSP streaming capability/specification.
- Keep the existing `enableStreaming` runtime switch, but make it enable WebRTC preview streaming.
- Reuse the existing native editor WebSocket endpoint for WebRTC signaling.
- Stream Phoenix's final framebuffer as H.264 WebRTC video when one or more browser peers are connected.
- Stream Phoenix's mixed demo audio alongside the video preview and mute Phoenix's local audio output while preview clients are receiving audio.
- Update Cacablu Preview to create a browser `RTCPeerConnection` and render the remote video track.

## Non-goals

- No changes to the existing TCP slave/editor ports.
- No section or asset administration changes.
- No TURN/STUN service requirement for the first localhost preview implementation.
- No remote-network optimization beyond localhost editor preview.
