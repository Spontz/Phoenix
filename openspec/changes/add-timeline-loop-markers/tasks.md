## 1. Cacablu Data Model

- [x] 1.1 Add `DbMarker` and include `markers` in the Cacablu project database model.
- [x] 1.2 Add a DB migration that creates `MARKERS(id, time, label)` for existing projects and migrates legacy lowercase `markers`.
- [x] 1.3 Extend the DB reader to load markers ordered by time and id.
- [x] 1.4 Add DbSession marker CRUD helpers for create, update time, update label, delete, and restore with id.
- [x] 1.5 Add unit tests for opening old databases, saving markers, deleting markers, and restoring markers.

## 2. Cacablu Timeline Interaction

- [x] 2.1 Render persisted markers as inverted triangles on the timeline ruler.
- [x] 2.2 Split ruler pointer handling into upper seek-only and lower loop-selection zones.
- [x] 2.3 Implement Shift+click marker creation at clicked time.
- [x] 2.4 Implement marker selection by clicking a triangle without changing bar selection behavior.
- [x] 2.5 Implement marker dragging to update marker time and refresh timeline/panel views.
- [x] 2.6 Implement Delete/Backspace deletion for selected markers.
- [x] 2.7 Push undo entries for marker create, drag/edit, and delete/restore.
- [x] 2.8 Add timeline styles for marker triangles, labels, vertical guide lines, hover state, selected state, drag state, glow, and pulse.
- [x] 2.9 Add double-click marker behavior that opens/focuses the Markers panel and selects the clicked marker.

## 3. Cacablu Markers Panel

- [x] 3.1 Add a Markers panel factory and register it in the panel registry.
- [x] 3.2 Add a Panels menu item that opens the Markers panel.
- [x] 3.3 List markers ordered by time in a searchable listbox with editable label and time fields.
- [x] 3.4 Persist label edits through DbSession and refresh timeline marker display.
- [x] 3.5 Persist valid time edits through DbSession, reject invalid numeric input, and refresh timeline marker positions.
- [x] 3.6 Ensure marker edits from the panel use the shared undo manager.

## 4. Cacablu Phoenix Loop Client

- [x] 4.1 Add a Phoenix runtime loop client for `PUT /api/runtime/loop`.
- [x] 4.2 Compute loop start/end from nearest marker boundaries with demo start/end fallback.
- [x] 4.3 On lower-zone ruler click, seek to the clicked demo time and send the computed active loop to Phoenix.
- [x] 4.4 Keep the selected active loop visible in Cacablu state without opening Events on errors.
- [x] 4.5 Mark loop API failures as Events without changing marker persistence.
- [x] 4.6 Render the active loop indicator only in the lower half of the time ruler.

## 5. Phoenix Runtime Loop API

- [x] 5.1 Add request/response structures for runtime loop updates in `EditorApiServer`.
- [x] 5.2 Add `PUT /api/runtime/loop` with CORS-compatible JSON handling.
- [x] 5.3 Validate finite `startTime`/`endTime` and reject `endTime <= startTime` with `400 Bad Request`.
- [x] 5.4 Queue accepted loop updates onto the runtime thread.
- [x] 5.5 Apply accepted loop start/end using Phoenix's existing loop start time and loop end time functions.
- [x] 5.6 Return the accepted loop interval and request id after successful application.
- [x] 5.7 Preserve the previous runtime loop if validation or runtime apply fails.

## 6. Verification

- [x] 6.1 Add Cacablu unit tests for marker DB migration and CRUD.
- [x] 6.2 Add Cacablu unit tests for loop interval calculation.
- [x] 6.3 Add Playwright coverage for creating, selecting, dragging, deleting, undoing, and panel-editing markers.
- [x] 6.4 Add Playwright coverage for Shift+click marker creation, lower-zone click selecting loop intervals, lower-only loop indicator rendering, and marker/demo-boundary fallback.
- [x] 6.5 Add Phoenix API tests or smoke coverage for valid and invalid `PUT /api/runtime/loop`.
- [x] 6.6 Run Cacablu typecheck/tests and the relevant Phoenix build/test target.
