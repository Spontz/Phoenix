## Why

Timeline-specific commands are currently grouped under a menu named `Bars`, which no longer describes the full scope of that menu. Authors also need a direct way to append an empty layer before creating or moving content onto it.

## What Changes

- Rename the top-level `Bars` menu to `Timeline` while preserving its existing actions and shortcuts.
- Add a `New Layer` action to the `Timeline` menu.
- Register `Ctrl+L` as the keyboard shortcut for `New Layer`.
- Append one empty timeline layer with the next available numeric layer index each time the command succeeds.
- Treat the complete visible Timeline surface as contiguous empty layers, so dragging in any visible row can create a bar even when that layer did not previously contain one.
- Keep newly added empty layers for the active project session; once a bar is placed on one, the existing bar layer value persists it naturally in the project database.
- Disable or ignore the command when no project or Timeline panel is available, and preserve native text-editor shortcuts.

## Capabilities

### New Capabilities

- `timeline-layer-command`: Timeline menu naming and the menu/keyboard workflow for appending an empty timeline layer.

### Modified Capabilities

None.

## Impact

- Cacablu menu action definitions, menu typing, and menu rendering.
- Application shortcut routing and menu enablement.
- Timeline panel track state, database refresh reconciliation, and rendering.
- Unit and Playwright coverage for menu naming, shortcut behavior, repeated layer creation, and project lifecycle.
- No Phoenix API, engine, SQLite schema, or dependency changes are required.
