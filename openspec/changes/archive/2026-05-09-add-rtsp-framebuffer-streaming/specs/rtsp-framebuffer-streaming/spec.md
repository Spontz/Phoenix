## ADDED Requirements

### Requirement: Explicit framebuffer streaming activation
The system SHALL provide an RTSP video stream of the engine's final OpenGL framebuffer when Phoenix is launched with the `enableStreaming` argument.

#### Scenario: Streaming starts from CLI argument
- **WHEN** Phoenix starts with the `enableStreaming` argument
- **THEN** the system opens an RTSP stream for the rendered framebuffer

#### Scenario: Streaming is disabled by default
- **WHEN** Phoenix starts without the `enableStreaming` argument
- **THEN** the system does not start RTSP framebuffer streaming by default

#### Scenario: Streaming is toggled from Engine Config
- **WHEN** the user toggles the RTSP streaming checkbox in the ImGui Engine Config panel
- **THEN** the system starts or stops RTSP framebuffer streaming to match the checkbox state

### Requirement: VLC-compatible RTSP endpoint
The system SHALL expose the framebuffer stream through a hardcoded RTSP URL compatible with VLC, using port 554.

#### Scenario: Default endpoint is available
- **WHEN** RTSP streaming starts
- **THEN** the stream is available on the standard RTSP port using the hardcoded stream path

#### Scenario: Client connects with VLC
- **WHEN** a VLC client opens the advertised RTSP URL
- **THEN** the client receives a live video stream of the engine output

### Requirement: Frame capture source
The system SHALL capture the final demo framebuffer image intended for presentation, not an arbitrary intermediate FBO and not debug ImGui overlays.

#### Scenario: Rendered frame is captured
- **WHEN** the engine completes rendering a frame
- **THEN** the RTSP stream receives the corresponding final framebuffer image

#### Scenario: Internal FBOs are used during rendering
- **WHEN** a demo renders through one or more internal FBOs before final composition
- **THEN** the stream contains the final composed output rather than the intermediate FBO contents

#### Scenario: Debug UI is enabled
- **WHEN** local debug ImGui overlays are enabled
- **THEN** the RTSP stream contains the demo framebuffer without the debug overlay UI

### Requirement: Window-sized stream resolution
The system SHALL encode the RTSP stream at the current window framebuffer resolution.

#### Scenario: Stream starts after window creation
- **WHEN** the first frame is streamed
- **THEN** the encoded video dimensions match the current window framebuffer dimensions, adjusted only as needed for encoder-compatible even dimensions

#### Scenario: Window is resized
- **WHEN** the Phoenix window changes size while streaming is active
- **THEN** subsequent streamed frames use the resized window framebuffer dimensions

### Requirement: Single client streaming
The system SHALL support one VLC-compatible RTSP client for the first implementation.

#### Scenario: First client connects
- **WHEN** no RTSP client is connected and a VLC client opens the advertised RTSP URL
- **THEN** the client receives the live framebuffer stream

#### Scenario: Additional client connects
- **WHEN** one RTSP client is already connected and another client attempts to connect
- **THEN** the system keeps the engine running and does not disrupt the active stream

### Requirement: Hardcoded streaming defaults
The system SHALL use standard hardcoded values for RTSP port, path, FPS, bitrate, and codec in the first implementation.

#### Scenario: No streaming value is required
- **WHEN** Phoenix is launched with the `enableStreaming` argument
- **THEN** the system starts RTSP streaming without requiring a value for that argument

#### Scenario: No SPO streaming configuration is required
- **WHEN** Phoenix loads SPO configuration
- **THEN** the system starts RTSP streaming without requiring RTSP-specific SPO variables

### Requirement: Non-disruptive streaming failure
The system MUST continue normal engine execution if RTSP streaming cannot start or fails during runtime.

#### Scenario: RTSP port is unavailable
- **WHEN** the configured RTSP port is already in use
- **THEN** the system logs the streaming startup failure and continues running the engine

#### Scenario: Client disconnects
- **WHEN** an RTSP client disconnects during playback
- **THEN** the system continues rendering and remains available for a later client connection when supported by the streaming backend

### Requirement: Bounded render impact
The system MUST avoid unbounded blocking of the render loop due to framebuffer streaming.

#### Scenario: Encoder or network output is slower than rendering
- **WHEN** streaming cannot consume frames at the engine's render rate
- **THEN** the system drops queued frames or reduces streaming throughput rather than indefinitely blocking the render loop

### Requirement: Clean shutdown
The system SHALL release RTSP, encoder, and framebuffer capture resources during engine shutdown.

#### Scenario: Engine exits while streaming
- **WHEN** Phoenix shuts down while RTSP streaming is active
- **THEN** the system closes the stream and releases streaming resources cleanly
