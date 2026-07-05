## Why

Cacablu needs a first-class Demo Settings panel for project-level controls that currently live only in Phoenix `control.spo`. The editor must calculate the correct demo end time from the timeline and let Phoenix apply and persist those settings consistently.

## What Changes

- Add a Phoenix editor API endpoint for reading and replacing demo control settings.
- Persist accepted settings to `data/config/control.spo`.
- Apply accepted settings to Phoenix runtime state: demo name, loop, sound, debug grid, log detail, and calculated demo end.
- Define supported log detail options from the current Phoenix `LogLevel` enum.
- Treat legacy `log_detail 4` files as invalid/legacy input that the new API normalizes to a supported value instead of exposing `4` in the UI.

## Capabilities

### New Capabilities

- `demo-settings-config`: Covers editor-driven demo control settings, runtime application, and `control.spo` persistence.

### Modified Capabilities

- Native editor API gains demo settings endpoints.

## Impact

- Phoenix editor API server and request routing.
- Phoenix demo control state and `data/config/control.spo` serialization.
- Cacablu Edit menu, floating Demo Settings panel, and project DB demo settings persistence.
- Timeline-derived demo end calculation from the latest bar end time.
