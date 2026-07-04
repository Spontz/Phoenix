## 1. Project Bar Persistence

- [ ] 1.1 Add project-session helpers for creating, updating, and deleting `BARS` rows.
- [ ] 1.2 Preserve stable bar ids and default metadata when creating a new timeline bar.
- [ ] 1.3 Add validation for positive duration, non-negative time, valid layer, and supported bar fields before database writes.
- [ ] 1.4 Add focused tests for bar create/update/delete persistence against an in-memory or copied SQLite project.

## 2. Timeline Editing UI

- [ ] 2.1 Keep Timeline openable and empty when no project is loaded.
- [ ] 2.2 Render timeline clips from project bars after project load without default placeholder layers.
- [ ] 2.3 Implement timeline bar selection and empty-space deselection.
- [ ] 2.4 Open/reinitialize Bar Editor on single-clicking a timeline bar, including the same selected bar.
- [ ] 2.5 Implement Bar Editor controls: Bar Type, Script Template, Save Template, code editor, Blend Source, Blend Destination, Blend Equation, and Apply.
- [ ] 2.6 Persist Bar Editor Apply changes to script, blend source, blend destination, and blend equation.
- [ ] 2.7 Restrict Blend Equation choices to Add, Subtract, and Reverse subtract while storing Phoenix-compatible values.
- [ ] 2.8 Add View menu toggle for displaying bar ids before timeline labels.
- [ ] 2.9 Implement bar creation from a timeline command or gesture.
- [ ] 2.10 Implement drag-to-move for changing bar start/end and layer without same-layer overlaps.
- [ ] 2.11 Implement resize handles for changing bar start or end time.
- [ ] 2.12 Implement delete for selected bars.
- [ ] 2.13 Implement undo stack support for committed bar moves.
- [ ] 2.14 Preserve layout stability and prevent label overlap during timeline edits and zoom changes.
- [ ] 2.15 Render section sync error bars in red while matching Events remain present.

## 3. Bar Editor, Inspector, And Selection

- [ ] 3.1 Extend app selection state to represent selected timeline bars.
- [ ] 3.2 Use Bar Editor as the primary selected-bar property editor.
- [ ] 3.3 Keep Inspector from treating timeline bars as resource files.
- [ ] 3.4 Keep timeline selection, Bar Editor state, Inspector state, and Events panel behavior consistent after bar deletion.
- [ ] 3.5 Keep Events text compact enough for dense section diagnostics.

## 4. Phoenix Synchronization

- [ ] 4.1 Add a debounced timeline edit sync scheduler that reuses project bar to Phoenix section sync.
- [ ] 4.2 Skip Phoenix sync when no project is loaded or when Phoenix is disconnected.
- [ ] 4.3 Record disconnected sync attempts as Events without rolling back local edits.
- [ ] 4.4 Record Phoenix section sync validation/load errors as Events with bar ids.
- [ ] 4.5 Ensure Cacablu sync progress counters advance only for real processed units, including section manifest checking, and do not reset during one-shot Phoenix requests.
- [ ] 4.6 Avoid sync loops by distinguishing local timeline edits from runtime state updates.
- [ ] 4.7 Confirm Phoenix section sync responses include failed section ids/messages and a manifest usable for exact-match checks.
- [ ] 4.8 Add/confirm Phoenix single-section sync rewrites only the moved bar `.spo` file.

## 5. Transport Behavior

- [ ] 5.1 Keep transport controls visible when Timeline is open without Phoenix.
- [ ] 5.2 Disable or no-op only the controls that require Phoenix while disconnected.
- [ ] 5.3 Keep playhead updates driven by Phoenix runtime state when connected.
- [ ] 5.4 Ensure local timeline editing does not force Phoenix playback state changes.
- [ ] 5.5 Add a playhead glow trail while playback advances and fade the trail when paused.

## 6. Validation

- [ ] 6.1 Verify opening Cacablu with no project shows no default bars or layers.
- [ ] 6.2 Verify project bars load into the timeline with correct timing and layers.
- [ ] 6.3 Verify create, move, resize, layer change, and delete persist to the project database.
- [ ] 6.4 Verify invalid edits are clamped or rejected before persistence.
- [ ] 6.5 Verify timeline edits trigger debounced Phoenix section sync when connected.
- [ ] 6.6 Verify timeline edits remain local and produce Events when Phoenix is disconnected.
- [ ] 6.7 Verify Phoenix sync errors identify the affected bar ids in Events.
- [ ] 6.8 Verify section sync error bars appear red in the timeline.
- [ ] 6.9 Verify sync modal progress text does not show stale `0/N` and the bar does not reset during one-shot Phoenix requests.
- [ ] 6.10 Verify Bar Editor single-click open, blend equation values, and Apply persistence.
- [ ] 6.11 Verify View > Display IDs toggles `<id> <name>` labels and changes to Ocultar IDs.
- [ ] 6.12 Verify moving a selected bar cannot overlap another bar and causes Phoenix to rewrite only the moved bar `.spo`.
- [ ] 6.13 Verify Edit > Undo restores the previous bar move through the undo stack.
