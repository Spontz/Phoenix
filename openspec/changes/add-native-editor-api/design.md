## Context

Phoenix runs demo playback and rendering in C++/OpenGL. It already has a slave-mode network path through `NetDriver`, using TCP ports for a legacy editor protocol. Cacablu is a browser-based editor and should connect directly to Phoenix without a backend. Browser JavaScript can use WebSocket and HTTP/fetch, but cannot use Phoenix's current raw TCP protocol directly.

The new API should therefore be native to Phoenix, browser-compatible, high performance, and scoped to slave mode.

## Goals / Non-Goals

**Goals:**
- Add a native HTTP/WebSocket editor API server to Phoenix.
- Start the new API only when `m_slaveMode` is active.
- Keep the existing TCP `NetDriver` behavior unchanged.
- Use a high-performance WebSocket-capable library, with uWebSockets as the intended choice.
- Allow Cacablu to connect directly from the browser using native `WebSocket` and `fetch`.
- Keep a persistent WebSocket connection open for runtime state, editor commands, logs/errors, and section/asset change events.
- Use HTTP for health checks, snapshots, and file-oriented operations.
- Support live playhead updates, playback control, section timeline management, and data-folder asset management.

**Non-Goals:**
- Removing, disabling, or replacing the current TCP editor connection.
- Adding a Cacablu backend bridge.
- Adding remote authentication, multi-user collaboration, or internet-facing hosting in the first iteration.
- Sending large asset file contents over WebSocket.
- Rewriting the existing section/event system.

## Decisions

### Add a dedicated editor API server

Create a new subsystem, for example `EditorApiServer`, instead of changing `NetDriver`.

Rationale: the current TCP protocol should remain untouched. A dedicated server keeps browser API concerns separate while still calling into the same Phoenix runtime objects.

### Start only in slave mode

Wire the new server next to the existing slave-mode network lifecycle in `DemoKernel`. When Phoenix is not in slave mode, the editor API does not listen on any port.

Rationale: this matches the existing editor workflow and keeps normal Phoenix runs free from extra network surfaces.

### Use uWebSockets for Phoenix

Use uWebSockets as the intended HTTP/WebSocket implementation because runtime performance is important and connections remain open while Phoenix streams state to Cacablu.

Rationale: the Phoenix side should minimize per-message overhead and avoid a thread-per-WebSocket model. If build integration proves unexpectedly costly, the implementation may document the issue and revisit the library choice before coding continues.

### Use browser-native APIs in Cacablu

Cacablu should use native browser `WebSocket` for the live channel and native `fetch` for HTTP endpoints.

Rationale: this avoids a backend process and keeps the client flexible.

### Split WebSocket and HTTP responsibilities

Use WebSocket for persistent, low-latency messages:
- `runtime.state`
- `runtime.play`
- `runtime.pause`
- `runtime.seek`
- `section.patch`
- `section.create`
- `section.delete`
- `asset.changed`
- `log`
- `error`

Use HTTP for snapshots and file-oriented operations:
- `GET /api/health`
- `GET /api/runtime`
- `GET /api/sections`
- `GET /api/assets`
- `PUT /api/assets/<relative-path>`
- `DELETE /api/assets/<relative-path>`

Rationale: timeline state and commands benefit from a persistent channel. Asset upload/delete and full snapshots fit HTTP better.

### Keep the first implementation minimal but extensible

Implement in phases:
1. Health endpoint and WebSocket connection.
2. Runtime state stream plus play/pause/seek.
3. Section snapshot and section mutation messages.
4. Asset listing and file mutation endpoints.

Rationale: the first visible integration should prove direct browser-to-Phoenix connectivity and dynamic playhead movement before expanding to the full editor surface.

### Treat Phoenix runtime as session authority

During an editing session, Cacablu should reflect Phoenix runtime state. Persistent `.spo`/asset files remain the project representation on disk.

Rationale: Cacablu's timeline must match what Phoenix is actually playing. Disk writes should be coordinated and followed by explicit reload/update notifications.

### Constrain data-folder access

Asset operations must be limited to paths inside Phoenix's configured `data` folder. Requests must use relative paths, reject traversal, and avoid arbitrary absolute filesystem access.

Rationale: the editor API can create, modify, and delete files. It must not become a general filesystem endpoint.

## Message Shape

Use JSON messages over WebSocket.

Runtime state example:

```json
{
  "type": "runtime.state",
  "time": 12.345,
  "playing": true,
  "fps": 60.0,
  "startTime": 0.0,
  "endTime": 180.0
}
```

Seek command example:

```json
{
  "type": "runtime.seek",
  "time": 42.0
}
```

Section patch example:

```json
{
  "type": "section.patch",
  "id": "logo",
  "patch": {
    "startTime": 10.0,
    "endTime": 15.5,
    "layer": 2
  }
}
```

Errors should also be structured JSON so Cacablu can show failures next to the relevant timeline or asset operation.

## Risks / Trade-offs

- uWebSockets build integration on Windows/CMake/vcpkg may require extra care.
- Streaming too many runtime messages can waste CPU; start with a bounded update rate and make it configurable later only if needed.
- Section updates must avoid duplicating existing `SectionEventManager` behavior; prefer reusing current section operations.
- File operations can corrupt project data if writes are not constrained and atomic.
- Browser origin/CORS behavior must be handled intentionally for local development.

## Migration Plan

1. Add the new editor API server behind slave mode.
2. Keep `NetDriver` unchanged.
3. Implement the smallest API surface needed for connection and runtime playhead streaming.
4. Add section management.
5. Add data-folder asset management.
6. Update Cacablu specs separately to consume the agreed HTTP/WebSocket contract from `main`.

## Open Questions

- Which port should be the default editor API port?
- Should the API bind to localhost only in the first implementation?
- What exact section serialization should Cacablu use for each section type beyond common timeline fields?
