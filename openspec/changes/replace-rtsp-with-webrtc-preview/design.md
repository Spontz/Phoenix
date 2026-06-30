## Architecture

Phoenix keeps `enableStreaming` as the operator-facing switch, but the implementation becomes a WebRTC media sender instead of an RTSP server. Cacablu remains browser-only and uses the existing `ws://127.0.0.1:29100/ws` connection for signaling.

## Phoenix

- Add `libdatachannel` as the lightweight C++ WebRTC dependency.
- Keep the current framebuffer capture and H.264 encoding flow where possible:
  - capture final framebuffer before ImGui overlays;
  - bound queue size to one frame;
  - encode with low-latency H.264 settings;
  - drop frames if the encoder or peer cannot keep up.
- Replace RTSP listener/session/RTP-over-TCP code with a WebRTC peer and video track.
- Accept WebRTC signaling messages on the editor WebSocket:
  - `webrtc.request`: browser request for a preview session;
  - `webrtc.offer`: Phoenix SDP offer for the send-only video track;
  - `webrtc.answer`: browser SDP answer;
  - `webrtc.ice-candidate`: optional candidate exchange if trickle ICE is needed;
  - `webrtc.state`: optional connection/stream status update.
- Only create WebRTC streaming resources when `enableStreaming` is active.

## Cacablu

- The Preview panel receives the existing connection controller.
- When Phoenix WebSocket is connected, Preview creates an `RTCPeerConnection`, sends a `webrtc.request`, answers Phoenix's `webrtc.offer`, and attaches the first remote video track to the preview `<video>`.
- If Phoenix disconnects or the panel is disposed, Preview closes the peer connection and clears the video element.
- Existing top-right connection status remains the single global Phoenix connection indicator.

## Performance Notes

- WebRTC should avoid the browser-incompatible RTSP path and remove the need for an external transcoder.
- The framebuffer queue remains bounded to prevent render-loop backpressure.
- For the first implementation, localhost host candidates are sufficient; STUN/TURN can be added later if remote browsing becomes a requirement.
