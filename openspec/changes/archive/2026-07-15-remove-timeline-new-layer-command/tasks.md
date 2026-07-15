## 1. Remove Explicit Layer Command

- [x] 1.1 Remove the `new-timeline-layer` menu action while preserving the `Timeline` menu and its remaining actions.
- [x] 1.2 Remove shell enablement, dispatch, and `Ctrl+L` interception for New Layer.
- [x] 1.3 Remove the Timeline panel command listener and cleanup path.

## 2. Simplify Timeline Layer State

- [x] 2.1 Remove `TimelineLayerSession` creation, dependency injection, and lifecycle clearing.
- [x] 2.2 Derive database-backed tracks and implicit surface extension directly from occupied bar layers.
- [x] 2.3 Remove the unused timeline-layer service and its explicit-layer unit tests.

## 3. Preserve Implicit Surface Behavior

- [x] 3.1 Rewrite the focused Playwright workflow without menu or shortcut assertions.
- [x] 3.2 Verify trusted drag-to-create works on an unused visible row and persists the numeric layer.
- [x] 3.3 Verify a full unused window, vertical scroll access, and grid coverage remain below the last occupied layer.
- [x] 3.4 Verify reopening the source project reconstructs only occupied plus implicit presentation layers.

## 4. Validation

- [x] 4.1 Run focused searches proving New Layer identifiers and `Ctrl+L` routing are gone.
- [x] 4.2 Run Cacablu typecheck, unit tests, targeted lint, production build, and Timeline Playwright regressions.
- [x] 4.3 Run strict OpenSpec validation and confirm no Phoenix or SQLite schema change is required.
