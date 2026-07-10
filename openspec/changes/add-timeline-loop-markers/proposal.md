## Why

Cacablu needs a fast way to define reusable time boundaries and select playback loops directly from the timeline. Phoenix already supports loop start/end internally, but there is no editor workflow or API contract for choosing loop intervals from timeline markers.

## What Changes

- Add persistent timeline markers stored in a new Cacablu database table named `MARKERS` with `id`, `time`, and `label`.
- Split the time ruler interaction into two vertical zones:
  - the upper zone seeks the active demo time without changing the active loop,
  - the lower zone seeks and selects the active loop interval.
- Add Shift+click marker creation on the ruler.
- Add inverted-triangle marker handles on the timeline ruler that can be selected, dragged, deleted, undone, edited, labeled, and opened in the Markers panel by double-clicking.
- Add a Markers panel available from the Panels menu, with quick search plus marker label/time editing.
- Define active-loop selection from the nearest marker to the left and right of the clicked time, falling back to demo start/end when a side has no marker.
- Send the selected active loop to Phoenix using its loop start/end controls.
- Add Phoenix HTTP endpoint support so Cacablu can update the active loop over the native editor API.

## Capabilities

### New Capabilities
- `timeline-loop-markers`: Persistent timeline markers, marker editing UI, loop interval selection, and Phoenix loop API integration.

### Modified Capabilities

## Impact

- Cacablu database schema and migrations for the new `MARKERS` table, including migration from legacy lowercase `markers`.
- Cacablu timeline ruler rendering, pointer interaction, undo/redo, keyboard deletion, and menu registration.
- Cacablu panel registry/menu actions for the new Markers panel.
- Cacablu Phoenix client code for active loop updates.
- Phoenix native editor HTTP API endpoint for setting loop start/end.
- Phoenix runtime loop control path that applies the requested loop interval during playback.
