## Purpose

Provide a browser-compatible low-latency preview of Phoenix's final OpenGL framebuffer so Cacablu can display the running engine directly in its Preview panel.

## Requirements

### Requirement: Explicit WebRTC preview activation
Phoenix SHALL enable framebuffer WebRTC preview streaming only when the existing streaming switch is enabled.

#### Scenario: Streaming starts from CLI argument
- **WHEN** Phoenix starts with the `enableStreaming` argument
- **THEN** Phoenix initializes the WebRTC preview streamer instead of an RTSP server

#### Scenario: Streaming is disabled by default
- **WHEN** Phoenix starts without the `enableStreaming` argument
- **THEN** Phoenix does not initialize preview streaming resources

#### Scenario: Streaming is toggled from Engine Config
- **WHEN** the user toggles the streaming checkbox in the ImGui Engine Config panel
- **THEN** Phoenix starts or stops WebRTC preview streaming to match the checkbox state

### Requirement: Browser WebRTC signaling through native editor API
Phoenix SHALL use the existing native editor WebSocket endpoint for WebRTC preview signaling.

#### Scenario: Browser requests a preview session
- **WHEN** Cacablu sends a `webrtc.request` message over `/ws`
- **THEN** Phoenix creates or replaces the preview peer connection and returns a `webrtc.offer` before sending any related ICE candidates

#### Scenario: Browser answers the preview offer
- **WHEN** Cacablu sends a `webrtc.answer` message over `/ws`
- **THEN** Phoenix applies the browser answer to the active preview peer connection

#### Scenario: Browser or Phoenix produces ICE candidates
- **WHEN** either peer produces a usable ICE candidate
- **THEN** the candidate is exchanged using `webrtc.ice-candidate` messages when needed

#### Scenario: Multiple editor WebSockets are connected
- **WHEN** Phoenix produces ICE candidates for a preview session
- **THEN** Phoenix sends those candidates only to the WebSocket that requested that preview session

#### Scenario: Unsupported streaming message is received
- **WHEN** Phoenix receives an invalid WebRTC signaling message
- **THEN** Phoenix sends an error message without stopping the engine

### Requirement: Preview video track
Phoenix SHALL send the final framebuffer as an H.264 WebRTC video track to the connected Cacablu preview peer.

#### Scenario: Preview connects
- **WHEN** Cacablu completes WebRTC negotiation with Phoenix
- **THEN** the Preview panel displays the engine framebuffer in a video element

#### Scenario: No preview peer is connected
- **WHEN** streaming is enabled but no WebRTC peer is ready
- **THEN** Phoenix continues rendering normally and avoids unnecessary network sends

### Requirement: Frame capture source
Phoenix SHALL capture the final demo framebuffer image intended for presentation, not an arbitrary intermediate FBO and not debug ImGui overlays.

#### Scenario: Rendered frame is captured
- **WHEN** the engine completes rendering a frame
- **THEN** the WebRTC preview receives the corresponding final framebuffer image

#### Scenario: Debug UI is enabled
- **WHEN** local debug ImGui overlays are enabled
- **THEN** the WebRTC preview contains the demo framebuffer without the debug overlay UI

### Requirement: Bounded render impact
Phoenix MUST avoid unbounded blocking of the render loop due to WebRTC preview streaming.

#### Scenario: Encoder or peer output is slower than rendering
- **WHEN** streaming cannot consume frames at the engine render rate
- **THEN** Phoenix drops queued preview frames rather than indefinitely blocking the render loop

### Requirement: Clean shutdown
Phoenix SHALL release WebRTC, encoder, and framebuffer capture resources during engine shutdown.

#### Scenario: Engine exits while streaming
- **WHEN** Phoenix shuts down while WebRTC preview streaming is active
- **THEN** Phoenix closes the peer connection and releases streaming resources cleanly
