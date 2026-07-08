## Context

Cacablu reads the `BARS.enabled` column into each `DbBar`, but timeline bars currently behave as publishable sections regardless of that flag. Project section synchronization serializes supported bars into Phoenix payloads and includes the enabled flag in the `.spo` text, which means a disabled project bar can still exist as a Phoenix runtime section and root `.spo` file.

The editor already has a Bars menu, selected-bar state (`bar` and `bars` resource selections), undo support for timeline edits, and Phoenix section APIs for single-section updates and many-section deletes. The change should reuse those paths so toggling enable behaves like another selected-bar edit rather than introducing a separate persistence or transport model.

## Goals / Non-Goals

**Goals:**

- Expose `Bars > Toggle Enable` with `Ctrl+D` and disable it when no bars are selected.
- Toggle all selected bars as one operation, persist the `BARS.enabled` changes, keep selection intact, and make the action undoable.
- Render disabled timeline bars distinctly enough that users can tell they remain in the project but are not active in Phoenix.
- Delete disabled bars from Phoenix, including their root `.spo` files.
- Send re-enabled bars to Phoenix as new or updated single sections using the existing update pathway.
- Exclude disabled bars from initial full project section replacement.

**Non-Goals:**

- Removing disabled bars from the project database or timeline.
- Adding a per-bar checkbox UI in the Bar Editor.
- Changing the SQLite schema.
- Changing Phoenix's runtime meaning of an `enabled 0` `.spo` section for non-editor-loaded demos.

## Decisions

1. **Disabled project bars are omitted from Cacablu-to-Phoenix section payloads.**

   Cacablu will filter disabled bars before section serialization. This makes the project editor contract explicit: disabled means not present in Phoenix. Alternative considered: continue sending `.spo` files with `enabled 0`. That keeps more legacy `.spo` semantics, but it violates the requested behavior that disabled bars are deleted together with their `.spo` from Phoenix.

2. **Toggle sync uses delete for newly disabled ids and single-section update for newly enabled ids.**

   The existing Phoenix client supports `deleteMany(ids)` and `replaceOne(section)`. Disabling selected bars should call delete for those ids when Phoenix is connected. Enabling selected bars should call the same single-bar sync used by Apply/move so Phoenix writes the `.spo` and loads it like a new section. Alternative considered: always perform a full replacement after toggle. That would be simpler but increases blast radius and risks touching unrelated sections.

3. **The toggle is a selected-bar command in app shell/menu routing.**

   Add a menu action under Bars and a keyboard shortcut handler in the same layer that handles `Select All`. The command should inspect `resourceSelection`, resolve existing selected bars from the active session, update `enabled`, dispatch timeline refresh, and run Phoenix sync side effects. Alternative considered: implement only in the Timeline panel. That would make the menu action depend on panel lifecycle and would fail when bars are selected but the panel has not mounted.

4. **Mixed selections are inverted per bar.**

   `Toggle Enable` flips each selected bar independently: enabled bars become disabled and disabled bars become enabled. Alternative considered: normalize all selected bars to one target state. The command is a true toggle, so every selected bar must change to the opposite of its own current state.

5. **Undo restores both database state and Phoenix state.**

   The undo entry should restore prior enabled values and then perform the matching Phoenix operations for bars whose state changed again. This keeps local and Phoenix views coherent after undo. If Phoenix is disconnected, undo remains a local database operation and no disconnected-sync error event is needed.

## Risks / Trade-offs

- **Risk: Disabled bars with unsupported types could produce confusing errors when re-enabled.** -> Reuse existing single-section sync issue handling so unsupported re-enabled bars are marked in Events like any other failed section sync.
- **Risk: Existing project-open sync may see stale Phoenix `.spo` files for disabled bars.** -> Initial full replacement must compare Phoenix only against enabled project bars and rely on replacement cleanup to remove extra disabled ids.
- **Risk: Ctrl+D can conflict with browser bookmarking in some environments.** -> Handle the shortcut only outside text-editing targets and prevent default when the app command runs.
- **Risk: Undo side effects can fail against Phoenix after local state is restored.** -> Keep local undo authoritative, record Phoenix sync errors, and preserve the restored project state for correction.
