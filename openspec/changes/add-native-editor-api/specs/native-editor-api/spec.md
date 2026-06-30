## ADDED Requirements

### Requirement: Slave-mode editor API lifecycle
Phoenix SHALL expose a native HTTP/WebSocket editor API only while running in slave mode.

#### Scenario: Slave mode starts editor API
- **WHEN** Phoenix starts with slave mode enabled
- **THEN** the native editor API server is initialized and begins listening

#### Scenario: Non-slave mode does not start editor API
- **WHEN** Phoenix starts without slave mode enabled
- **THEN** the native editor API server does not listen on any port

#### Scenario: Existing TCP editor networking remains unchanged
- **WHEN** Phoenix starts in slave mode
- **THEN** the existing TCP `NetDriver` behavior remains available as before
- **AND** the native editor API starts in addition to it

### Requirement: Browser-compatible direct connection
Phoenix SHALL allow Cacablu to connect directly from a browser without a Cacablu backend bridge.

#### Scenario: Browser opens WebSocket
- **WHEN** Cacablu opens a browser-native WebSocket connection to Phoenix
- **THEN** Phoenix accepts the connection on the editor API WebSocket endpoint

#### Scenario: Browser uses HTTP fetch
- **WHEN** Cacablu sends browser-native HTTP requests to Phoenix editor API endpoints
- **THEN** Phoenix returns HTTP responses suitable for browser `fetch`

### Requirement: Health endpoint
Phoenix SHALL expose an HTTP health endpoint for editor connection discovery and diagnostics.

#### Scenario: Health is requested
- **WHEN** a client requests `GET /api/health`
- **THEN** Phoenix responds with a successful HTTP status and structured health information

### Requirement: Runtime state streaming
Phoenix SHALL stream runtime state over WebSocket so Cacablu can display a dynamic timeline playhead.

#### Scenario: Runtime state is streamed while connected
- **WHEN** Cacablu is connected to the WebSocket endpoint
- **THEN** Phoenix periodically sends structured runtime state messages

#### Scenario: Runtime state includes timeline time
- **WHEN** Phoenix sends a runtime state message
- **THEN** the message includes the current demo time
- **AND** the message includes playback status
- **AND** the message includes FPS, start time, and end time when available

#### Scenario: Runtime stream is bounded
- **WHEN** Phoenix streams runtime state messages
- **THEN** the update rate is bounded to avoid unnecessary runtime overhead

### Requirement: Runtime playback commands
Phoenix SHALL accept runtime playback commands over WebSocket from Cacablu.

#### Scenario: Play command is received
- **WHEN** Cacablu sends a `runtime.play` message
- **THEN** Phoenix starts or resumes demo playback using existing runtime behavior

#### Scenario: Pause command is received
- **WHEN** Cacablu sends a `runtime.pause` message
- **THEN** Phoenix pauses demo playback using existing runtime behavior

#### Scenario: Seek command is received
- **WHEN** Cacablu sends a `runtime.seek` message with a valid target time
- **THEN** Phoenix moves the demo current time to that target using existing runtime behavior

### Requirement: Remote preview mouse input
Phoenix SHALL accept mouse input messages over the editor WebSocket so Cacablu can interact with the streamed preview.

#### Scenario: Preview pointer is moved
- **WHEN** Cacablu sends an `input.mouse.move` message with framebuffer coordinates
- **THEN** Phoenix forwards the movement through the existing mouse event path
- **AND** Phoenix makes the position available to ImGui for the next frame

#### Scenario: Preview button is pressed or released
- **WHEN** Cacablu sends `input.mouse.down` or `input.mouse.up` with framebuffer coordinates and a valid button
- **THEN** Phoenix forwards the button event through the existing mouse event path
- **AND** Phoenix forwards the event to ImGui so debug panels and menus can be operated through the preview

#### Scenario: Preview wheel is scrolled
- **WHEN** Cacablu sends an `input.mouse.wheel` message with framebuffer coordinates and wheel deltas
- **THEN** Phoenix forwards the scroll event through the existing mouse event path
- **AND** Phoenix forwards the wheel event to ImGui

#### Scenario: Preview key is pressed or released
- **WHEN** Cacablu sends `input.key.down` or `input.key.up` with a valid GLFW key code
- **THEN** Phoenix forwards the key event through the existing keyboard event path
- **AND** Phoenix forwards the key event to ImGui

### Requirement: Section snapshot
Phoenix SHALL expose the current section timeline as structured data for Cacablu.

#### Scenario: Sections are requested
- **WHEN** Cacablu requests `GET /api/sections`
- **THEN** Phoenix responds with a structured list of current sections

#### Scenario: Section timeline fields are included
- **WHEN** Phoenix serializes a section for Cacablu
- **THEN** the serialized section includes id, type, start time, end time, duration, layer, and enabled state when available

### Requirement: Section management
Phoenix SHALL allow Cacablu to create, update, and delete sections through the native editor API.

#### Scenario: Section is created
- **WHEN** Cacablu sends a valid section creation request
- **THEN** Phoenix creates or loads the section using existing section loading/event behavior where possible

#### Scenario: Section is patched
- **WHEN** Cacablu sends a valid section patch request
- **THEN** Phoenix applies supported section timeline changes
- **AND** Phoenix emits a section change event to connected WebSocket clients

#### Scenario: Section is deleted
- **WHEN** Cacablu sends a valid section delete request
- **THEN** Phoenix removes the section using existing section management behavior
- **AND** Phoenix emits a section change event to connected WebSocket clients

### Requirement: Data folder asset listing
Phoenix SHALL expose files under the configured `data` folder as structured asset data for Cacablu.

#### Scenario: Assets are listed
- **WHEN** Cacablu requests `GET /api/assets`
- **THEN** Phoenix responds with a structured list of files under the configured `data` folder

### Requirement: Data folder asset mutation
Phoenix SHALL allow Cacablu to create, replace, and delete files under Phoenix's configured `data` folder.

#### Scenario: Asset is created or replaced
- **WHEN** Cacablu sends a valid asset write request for a relative path inside `data`
- **THEN** Phoenix writes the file under `data`
- **AND** Phoenix emits an asset change event to connected WebSocket clients

#### Scenario: Asset is deleted
- **WHEN** Cacablu sends a valid asset delete request for a relative path inside `data`
- **THEN** Phoenix deletes the file under `data`
- **AND** Phoenix emits an asset change event to connected WebSocket clients

#### Scenario: Asset path escapes data folder
- **WHEN** Cacablu sends an asset request with an absolute path or path traversal
- **THEN** Phoenix rejects the request
- **AND** no file outside `data` is modified

### Requirement: Structured error reporting
Phoenix SHALL return structured errors for invalid editor API requests.

#### Scenario: Unsupported WebSocket message is received
- **WHEN** Cacablu sends a WebSocket message with an unsupported type
- **THEN** Phoenix responds with a structured error message

#### Scenario: HTTP request fails validation
- **WHEN** Cacablu sends an invalid HTTP request
- **THEN** Phoenix responds with an appropriate HTTP error status and structured error details
