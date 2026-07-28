## 1. Menu And Command Wiring

- [x] 1.1 Add `Toggle Enable` to the Bars menu with shortcut `Ctrl+D`.
- [x] 1.2 Disable `Toggle Enable` when the current resource selection contains no existing bars.
- [x] 1.3 Add a `Ctrl+D` keyboard handler that ignores text-editing targets and invokes the same command as the menu.
- [x] 1.4 Keep selected bar ids selected after the toggle command runs.

## 2. Database State And Undo

- [x] 2.1 Add a project session helper or command path that updates `BARS.enabled` for a set of bar ids and updates in-memory `DbBar` objects.
- [x] 2.2 Implement per-bar inversion for mixed selections so each selected bar toggles from its current enabled state.
- [x] 2.3 Add an undo entry that restores each affected bar's previous enabled value.
- [x] 2.4 Dispatch the existing timeline refresh event after toggle and after undo.

## 3. Phoenix Synchronization

- [x] 3.1 Update section collection so disabled bars are omitted from Phoenix payloads and unsupported-type validation.
- [x] 3.2 On disable, call Phoenix `deleteMany` for the newly disabled selected bar ids when connected.
- [x] 3.3 On enable, call the existing single-bar Phoenix sync path for each newly enabled selected bar when connected.
- [x] 3.4 Keep local toggle changes when Phoenix sync fails and record section sync errors through the existing Events/error marker flow.
- [x] 3.5 Ensure disconnected toggles skip Phoenix requests without disconnected-sync error events.
- [x] 3.6 Ensure initial project-open section sync compares and replaces only enabled bars, removing stale Phoenix sections/files for disabled ids through replacement cleanup.

## 4. Timeline Presentation

- [x] 4.1 Render disabled timeline bars with a distinct visual state while keeping them visible and selectable.
- [x] 4.2 Ensure disabled bars still participate in selection, box selection, movement, deletion, and Bar Editor opening.
- [x] 4.3 Ensure section error styling remains visible or meaningfully resolved for disabled bars after Phoenix deletion.

## 5. Tests And Verification

- [x] 5.1 Add unit tests for `collectPhoenixSections` omitting disabled bars and not reporting unsupported disabled bar types.
- [x] 5.2 Add unit or integration tests for the menu action disabled/enabled state based on bar selection.
- [x] 5.3 Add tests for toggling one enabled bar, one disabled bar, and a mixed multi-selection.
- [x] 5.4 Add tests for Phoenix sync calls: delete on disable, update on enable, and no request while disconnected.
- [x] 5.5 Add tests that undo restores enabled values and triggers the matching Phoenix sync side effects when connected.
- [x] 5.6 Run Cacablu typecheck and relevant unit tests.
- [x] 5.7 Verify project-open section sync sends bars by layer and normalizes script line endings before Phoenix parsing.
