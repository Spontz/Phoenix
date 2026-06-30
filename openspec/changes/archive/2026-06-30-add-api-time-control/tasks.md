## 1. Build and Lifecycle

- [x] 1.1 Add the selected HTTP/WebSocket dependency for the native editor API, targeting uWebSockets.
- [x] 1.2 Create a dedicated editor API server module without modifying `NetDriver` behavior.
- [x] 1.3 Initialize the editor API server only when Phoenix is running in slave mode.
- [x] 1.4 Update the editor API server from the existing runtime loop while slave mode is active.
- [x] 1.5 Shut down the editor API server cleanly during Phoenix shutdown.

## 2. Initial API Surface

- [x] 2.1 Add `GET /api/health`.
- [x] 2.2 Add WebSocket endpoint `/ws`.
- [x] 2.3 Accept browser WebSocket connections from Cacablu.
- [x] 2.4 Send structured JSON errors for malformed or unsupported WebSocket messages.

## 3. Runtime State and Control

- [x] 3.1 Stream `runtime.state` messages containing current demo time, playback status, FPS, start time, and end time.
- [x] 3.2 Bound the runtime state update rate so the stream remains performant.
- [x] 3.3 Handle `runtime.play`.
- [x] 3.4 Handle `runtime.pause`.
- [x] 3.5 Handle `runtime.seek`.
- [x] 3.6 Ensure seek/play/pause commands reuse existing `DemoKernel` behavior.
- [x] 3.7 Handle `runtime.toggle` for robust play/pause transport control.
- [x] 3.8 Handle `input.mouse.move`, `input.mouse.down`, `input.mouse.up`, and `input.mouse.wheel`.
- [x] 3.9 Forward remote preview mouse input through Phoenix's existing event path and ImGui.
- [x] 3.10 Handle `input.key.down` and `input.key.up` for focused preview keyboard input.

## 4. Section Timeline API

- [ ] 4.1 Add `GET /api/sections` returning a snapshot of current Phoenix sections.
- [ ] 4.2 Include common timeline fields: id, type, start time, end time, duration, layer, enabled state, and source where available.
- [ ] 4.3 Handle `section.create` by reusing the existing section loading/event path where possible.
- [ ] 4.4 Handle `section.patch` for start time, end time, layer, enabled state, and source/body update where supported.
- [ ] 4.5 Handle `section.delete`.
- [ ] 4.6 Emit section change events over WebSocket after successful mutations.

## 5. Data Folder Asset API

- [ ] 5.1 Add `GET /api/assets` for listing files under Phoenix's configured `data` folder.
- [ ] 5.2 Add `PUT /api/assets/<relative-path>` for creating or replacing files under `data`.
- [ ] 5.3 Add `DELETE /api/assets/<relative-path>` for deleting files under `data`.
- [ ] 5.4 Reject absolute paths and path traversal.
- [ ] 5.5 Use safe writes for asset updates so partial writes do not corrupt existing files.
- [ ] 5.6 Emit asset change events over WebSocket after successful create/update/delete operations.

## 6. Validation

- [x] 6.1 Build Phoenix after adding the dependency and server module.
- [ ] 6.2 Run Phoenix outside slave mode and verify the new editor API does not listen.
- [ ] 6.3 Run Phoenix in slave mode and verify the existing TCP editor connection behavior is unchanged.
- [x] 6.4 Connect from a browser WebSocket client and verify live `runtime.state` updates.
- [x] 6.5 Verify Cacablu can use browser-native `fetch` against HTTP endpoints.
- [x] 6.6 Verify play, pause, and seek affect the Phoenix runtime correctly.
- [ ] 6.7 Verify section timeline edits update Phoenix without restarting the engine.
- [ ] 6.8 Verify asset create/update/delete operations remain confined to `data`.
