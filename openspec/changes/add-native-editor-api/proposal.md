## Why

Cacablu needs to connect directly from the browser to a running Phoenix instance in slave mode. The integration must show Phoenix runtime time continuously, allow playback control and timeline editing, and coordinate changes to files under Phoenix's `data` folder without requiring a Cacablu backend.

Phoenix already has a legacy TCP editor connection used in slave mode. This change adds a native high-performance HTTP/WebSocket editor API alongside the existing TCP behavior, without replacing or disabling the current `NetDriver` path.

## What Changes

- Add a native editor API server to Phoenix using a high-performance HTTP/WebSocket library, with uWebSockets as the intended implementation choice.
- Start the new API only when Phoenix is running in slave mode, matching the existing editor-network activation model.
- Keep the existing TCP editor connection behavior unchanged.
- Expose a WebSocket endpoint for persistent runtime state streaming and low-latency editor commands.
- Expose HTTP endpoints for health checks, snapshots, section listing, and `data` folder asset operations.
- Stream the current demo time/status to connected Cacablu clients so the timeline playhead can move dynamically.
- Accept playback and seek commands from Cacablu.
- Support section management from Cacablu so sections can be represented and edited as timeline bars.
- Support safe management of files under Phoenix's `data` folder from Cacablu.

## Capabilities

### New Capabilities

- `native-editor-api`: Covers slave-mode API lifecycle, WebSocket runtime streaming, HTTP snapshots, playback commands, section management, and `data` folder asset operations for browser-based editors.

### Modified Capabilities

None.

## Impact

- Affected runtime code: `DemoKernel` slave-mode networking lifecycle, new editor API server subsystem, `SectionManager`/`SectionEventManager` integration, and data-folder file access helpers.
- Build configuration: add the selected HTTP/WebSocket dependency and link it into the engine.
- Network behavior: Phoenix opens an additional local editor API port only in slave mode.
- Browser integration: Cacablu can connect directly with native `WebSocket` and `fetch`, without a backend bridge.
- Existing editor TCP behavior remains present and unchanged.
