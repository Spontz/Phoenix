## Why

The Timeline now exposes a continuous implicit layer surface, so a separate `New Layer` command no longer adds useful authoring capability. Keeping the menu action, `Ctrl+L`, and session-only explicit layer state makes the UI and implementation more complex while duplicating the existing drag-to-create workflow.

## What Changes

- **BREAKING** Remove `Timeline > New Layer` and its `Ctrl+L` shortcut.
- Remove session-only explicitly added empty layers and their command routing.
- Preserve the `Timeline` menu and all other Timeline actions.
- Preserve continuous implicit layers, the full unused window below the last occupied layer, vertical scrolling, grid rendering, and drag-to-create behavior.
- Simplify tests so they validate implicit layer creation without relying on the removed command.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `timeline-layer-command`: Remove the explicit empty-layer command and its lifecycle requirements while retaining the implicit Timeline layer surface.

## Impact

- Cacablu menu definitions and application shortcut routing.
- Timeline panel construction and implicit layer reconciliation.
- Timeline layer helpers and their unit/Playwright coverage.
- No Phoenix API, C++ engine, dependency, or SQLite schema changes.
