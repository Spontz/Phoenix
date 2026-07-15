## 1. Timeline Menu Contract

- [x] 1.1 Rename the menu type, menu order, and existing timeline action definitions from `Bars` to `Timeline`, and update the existing menu regression test.
- [x] 1.2 Add a stable `new-timeline-layer` menu action labeled `New Layer` under `Timeline` with the displayed shortcut `Ctrl+L`.
- [x] 1.3 Disable New Layer when no project is open or the Timeline panel is unavailable, without changing enablement of existing Timeline actions.

## 2. Session Layer State

- [x] 2.1 Add focused Timeline helpers that union database bar layers with explicitly added session layers and allocate `0` or `max + 1` deterministically.
- [x] 2.2 Retain session-added empty layers across ordinary Timeline rerenders and database-backed track rebuilds.
- [x] 2.3 Clear session-added empty layers when the project session closes or changes, while allowing occupied layers to reconstruct from persisted bars.
- [x] 2.4 Render each newly added layer immediately as an empty lane that participates in existing create and drag layer validation.
- [x] 2.5 Materialize contiguous implicit layers with at least one full panel-height window of unused layers below the last occupied or explicitly added layer, and reconcile them when the panel grows.
- [x] 2.6 Resolve drag-to-create gestures anywhere on that surface to the row's numeric layer, without requiring an explicit layer command or existing bar.
- [x] 2.7 Keep implicit surface layers out of logical allocation so `New Layer` still selects the next database-backed or session-added layer.
- [x] 2.8 Size the vertical time grid to the full rendered lane height so its guides remain visible during vertical scrolling.

## 3. Command Routing

- [x] 3.1 Route the New Layer menu action through one shell command that dispatches to the mounted Timeline panel.
- [x] 3.2 Route non-text `Ctrl+L` through the same command, preventing default only when the command can execute.
- [x] 3.3 Preserve native/editor handling for `Ctrl+L` in Monaco, inputs, textareas, selects, and contenteditable elements.
- [x] 3.4 Register and clean up the Timeline command event listener with the panel lifecycle.

## 4. Verification

- [x] 4.1 Add unit tests for Timeline menu naming, New Layer placement, label, shortcut, and enablement.
- [x] 4.2 Add unit tests for first-layer allocation, repeated allocation, numeric gaps, database/session layer union, rerender retention, and project-session reset.
- [x] 4.3 Add integration coverage proving menu and keyboard commands append exactly one layer and share identical numbering behavior.
- [x] 4.4 Verify text-editing contexts do not add layers and hosts that reserve `Ctrl+L` retain a functional menu action.
- [x] 4.5 Run Cacablu typecheck, unit/integration tests, targeted lint, production build, and strict OpenSpec validation; confirm no Phoenix or SQLite schema change is required.
- [x] 4.6 Extend the Playwright workflow to create a bar through a trusted pointer drag near the bottom of the implicit visible surface and verify its layer and time interval.
- [x] 4.7 Re-run Cacablu validation and strict OpenSpec validation after the interaction fix.
- [x] 4.8 Extend the Playwright scroll check to verify the grid covers the last reachable lane after scrolling.
- [x] 4.9 Re-run targeted Cacablu validation and strict OpenSpec validation after the grid fix.
