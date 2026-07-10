## Context

Cacablu owns the project timeline database and already renders a ruler above timeline bars. Phoenix owns runtime playback state and already has internal loop start/end controls, but those controls are not exposed through the editor HTTP API. Existing editor APIs live in `EditorApiServer` under `/api/...`, while Cacablu panels are registered through the panel registry and Panels menu.

The new workflow spans both repositories: Cacablu must persist marker definitions and expose marker editing interactions, while Phoenix must accept an active loop interval and apply it to runtime playback.

## Goals / Non-Goals

**Goals:**
- Persist timeline markers in Cacablu in a new `MARKERS` table with `id`, `time`, and `label`, migrating legacy lowercase `markers` tables when present.
- Render marker handles as inverted triangles on the timeline ruler.
- Split the ruler vertically so the upper zone seeks time only and the lower zone selects the active playback loop.
- Create markers with Shift+click on the ruler, while selected marker handles continue to support selection, dragging, and deletion.
- Add a Markers panel with quick search for editing marker labels and times.
- Support marker deletion with Delete/Backspace and undo.
- Send every selected active loop interval to Phoenix through a new HTTP endpoint.
- Apply the loop interval in Phoenix so playback remains inside the selected loop once it reaches it.

**Non-Goals:**
- Persist the active loop interval in Phoenix config files.
- Add marker import/export formats outside the SQLite project database.
- Add marker grouping, colors, or per-marker metadata beyond label and time.
- Change existing bar selection semantics.

## Decisions

### Store markers as first-class project rows

Cacablu will add a `MARKERS` table with `id INTEGER PRIMARY KEY`, `time REAL NOT NULL`, and `label TEXT NOT NULL DEFAULT ""`. If an older database contains a lowercase `markers` table, Cacablu will migrate its rows into `MARKERS`. The DB reader/session model will expose markers alongside bars, files, folders, variables, and FBOs.

Alternative considered: store markers in variables as serialized JSON. That would avoid a schema table but would make individual edits harder to query, undo, inspect in the DB Explorer, and migrate safely.

### Keep active loop as runtime state, not marker state

Markers define candidate boundaries. The active loop is computed when the user clicks the lower half of the ruler: nearest marker at or before the clicked time becomes the loop start, nearest marker at or after the clicked time becomes the loop end, with demo start/end fallback. The upper half of the ruler seeks time without changing the active loop. Cacablu then sends the resulting lower-zone loop interval to Phoenix.

Alternative considered: mark two markers as "active". That introduces persistent state the user did not request and complicates fallback intervals before the first marker and after the last marker.

### Use the existing timeline ruler as the interaction surface

The timeline ruler will become two hit zones. The upper half selects playback time only; the lower half selects playback time and active loop interval. Marker creation uses Shift+click on the ruler so normal lower-zone clicks cannot accidentally create marker boundaries. Marker handles will be rendered as inverted triangles positioned by marker time, with labels rendered to the right and vertical guide lines drawn through the timeline.

Alternative considered: add a separate marker lane. That would consume vertical space and make the relationship between time ticks and markers less direct.

### Add a dedicated Markers panel

The Markers panel will provide quick search, list markers ordered by time, and allow editing label/time values. Time edits use the same persistence and undo pathway as drag edits so timeline and panel behavior stay consistent. Double-clicking a marker triangle opens or focuses this panel and selects that marker.

Alternative considered: edit marker labels inline on the timeline. This is less efficient for batch edits and adds text editing complexity inside a dense ruler.

### Add `PUT /api/runtime/loop`

Phoenix will expose a browser-compatible editor endpoint that validates `startTime` and `endTime`, applies them through the existing runtime loop start/end functions, and returns the accepted interval. The endpoint will not persist to `control.spo`; demo settings remain responsible for persistent global loop enable/end settings.

Alternative considered: extend `PUT /api/demo-settings`. That endpoint is for project-level config persistence and would mix transient active-loop selection with saved demo settings.

## Risks / Trade-offs

- Marker clicks could conflict with existing ruler seeking -> require Shift+click for marker creation, split the ruler into explicit vertical hit zones for seek versus loop selection, and cover with Playwright tests for upper/lower behavior.
- Dragging markers can reorder them or collapse intervals -> sort markers by time for lookup and reject non-finite times while allowing equal times only if the implementation can render/select them predictably.
- Phoenix loop application may race playback updates -> apply loop changes on the runtime thread using the same request queue pattern as demo settings.
- Existing projects lack the `markers` table -> migration creates an empty table and readers tolerate older files.
- Undo across panel and timeline edits can diverge -> route create/update/delete marker operations through shared DbSession helpers and push undo entries from both UI surfaces.

## Migration Plan

1. Add the Cacablu DB migration that creates `markers` when absent.
2. Extend DB schema/reader/session APIs for marker CRUD.
3. Implement Cacablu marker rendering and interaction behind the migrated model.
4. Add the Markers panel and menu entry.
5. Add Phoenix runtime loop endpoint and client integration.
6. Verify existing projects open with no markers and existing timeline/bar workflows behave unchanged.
