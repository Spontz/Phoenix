## Why

Project bars already carry an enabled/disabled state, but the editor does not expose a direct workflow for toggling it or defining how disabled bars are represented in Phoenix. Users need a way to temporarily disable selected bars without deleting them from the project, while ensuring Phoenix no longer runs or keeps stale `.spo` files for disabled bars.

## What Changes

- Add a Bars menu action named `Toggle Enable` with shortcut `Ctrl+D`.
- Enable the action only when one or more timeline bars are selected.
- Toggle the enabled state of all selected bars when the action runs.
- Keep disabled bars in the project database and timeline.
- Treat disabled bars as absent from Phoenix: remove their runtime section and root `.spo` file.
- Treat re-enabled bars as newly sent sections: publish them to Phoenix using the existing single-section sync pathway where possible.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `timeline-management`: add selected-bar enabled toggling through the Bars menu and keyboard shortcut.
- `section-bar-sync`: define Phoenix synchronization semantics for disabled and re-enabled bars.

## Impact

- Cacablu menu registration, command routing, keyboard shortcuts, selection state, timeline rendering, undo/redo behavior, and project persistence.
- Cacablu bar-to-section synchronization logic so disabled bars are deleted from Phoenix and skipped during project-open replacement.
- Phoenix editor section delete/update endpoints and `.spo` persistence semantics as exercised by Cacablu.
- Tests for selected multi-bar toggling, disabled menu state, database persistence, and Phoenix sync side effects.
