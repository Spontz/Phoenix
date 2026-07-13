## Context

Cacablu's Pool browser renders database-backed `FOLDERS` and `FILES` rows. It already imports external drops, moves individual files by drag-and-drop, updates the SQLite session, and mirrors enabled paths to Phoenix. Selection is shared through `AppState`, but Cut, Copy, and Paste are currently routed by the shell to `document.execCommand`, so a selected Pool item has no clipboard behavior.

The requested workflow has two distinct destinations. A paste back into Cacablu must reproduce or move the selected database item, including recursive folder contents. A paste into Monaco or another text editor must insert a Phoenix-facing path beginning with `/pool`. The Clipboard API models these as representations of one logical item, but web browsers guarantee only a limited set of operating-system formats. In particular, Cacablu cannot rely on a browser to publish Windows Explorer's native filesystem file-list format.

## Goals / Non-Goals

**Goals:**

- Make keyboard shortcuts and Edit menu Cut, Copy, and Paste operate on one or more selected Pool items when the Pool browser owns the command.
- Add conventional Ctrl/Cmd toggle and Shift range selection semantics to the Pool tree for clipboard operations.
- Preserve native clipboard editing in Monaco, inputs, textareas, selects, and contenteditable elements.
- Publish a normalized `/pool/...` plain-text representation for pasting into any text destination.
- Copy files and complete folder subtrees within the project, and move cut files/folders only after a valid internal paste.
- Preflight each mutation so invalid moves and name conflicts cannot produce partial local changes.
- Keep the project database authoritative and reconcile affected enabled files with Phoenix through existing scoped APIs.
- Show pending-cut roots at 50% opacity until the cut is consumed or invalidated.

**Non-Goals:**

- Guarantee pasting arbitrary project files directly into Windows Explorer, the Desktop, or another native file manager from the web build.
- Overwrite or merge an existing destination item automatically.
- Move an item merely because its path was pasted into a text destination.
- Add a Phoenix endpoint or modify Phoenix C++ code.
- Make clipboard mutations undoable when equivalent existing drag/drop mutations are not yet integrated with Undo.

## Decisions

### Use an application clipboard snapshot plus system plain text

A small clipboard service stores an immutable asset snapshot with an operation (`copy` or `cut`), the source project/session identity, source ids and paths, and recursively captured folder/file metadata. The system clipboard receives normalized absolute Pool paths as `text/plain`, for example `/pool/textures/hero.png`, with one path per line when multiple roots are selected. If both a folder and one of its descendants are selected, snapshot canonicalization keeps only the folder root so the descendant is not copied twice.

For copy, file bytes and subtree metadata are captured so repeated internal pastes do not depend on later source edits. For cut, stable source identity is retained and revalidated immediately before paste; the source remains untouched until a valid internal destination accepts the move.

Alternative considered: rely only on a custom clipboard MIME type. Web custom formats are not guaranteed to survive into native applications or even every browser context, so they cannot be the source of truth for internal moves. An optional custom representation may be written when supported, but the application snapshot and `text/plain` remain authoritative.

### Route commands by focused context

The shell stops treating every non-text Cut/Copy/Paste shortcut as a generic `execCommand`. It first allows editable targets to retain native behavior. Otherwise it dispatches an application clipboard command that the focused/active Pool panel can claim. Menu actions use the same command path so keyboard and menu behavior cannot diverge.

The Pool panel claims Copy or Cut only with a valid asset selection. Ctrl/Cmd-click toggles individual rows and Shift-click extends a visible-tree range from the selection anchor. It claims Paste when an application asset snapshot exists and resolves the destination as the sole selected folder, the parent of the sole selected file, or the Pool root when nothing is selected. With an ambiguous multi-selection, Paste is disabled until the user selects one destination. Unclaimed commands retain the existing fallback behavior.

The browser renders the Pool root as an explicit row above the project tree. Selecting that row clears the item selection, making the existing no-selection destination resolve to parent id `0`. The same row and its root subtree are a drop target with path `pool`, so internal file moves and external file imports use the established mutation paths without introducing a second root-specific operation.

Alternative considered: attach only `copy`, `cut`, and `paste` DOM listeners to the tree. Dockview focus and menu invocation make ownership less reliable, while a shared command router can test active panel and editable targets explicitly.

### Treat external text paste and internal asset paste independently

Copy and Cut publish `/pool/...` as plain text. Text destinations consume that representation normally. They do not notify Cacablu that a paste completed, so a pending cut is never consumed by external paste. Only a successful Pool paste clears a cut snapshot. Copy snapshots remain available for repeated internal pastes.

The Pool publishes `text/plain` directly through the native `copy` or `cut` event produced by the keyboard shortcut or Edit menu command. Cacablu does not cancel Ctrl/Cmd+C or Ctrl/Cmd+X at `keydown`, because starting a second nested copy command from that handler is not reliable in every browser or local HTML context. The asynchronous Clipboard API remains a fallback when a command has no native data transfer. This ensures a subsequent native paste in Monaco sees the Pool path instead of stale operating-system clipboard contents.

Paths always use forward slashes, begin with `/pool`, contain no duplicate separators, and have no trailing slash. A selected folder yields its folder path, not the paths of all descendants; multiple canonical roots are emitted in visible-tree order, separated by the platform newline.

File drags retain their private Cacablu data-transfer representation for internal Pool moves and also expose the normalized path as `text/plain`. Pool folder targets explicitly choose the move effect, while text editors consume the plain-text representation as a copy, so dropping into Monaco inserts the reference without changing the project tree.

Alternative considered: delete a cut source after any paste event. That would lose project data when a user merely inserts the path into a script.

### Preflight recursive operations before mutating SQLite

Clipboard helpers build all source trees and one destination plan before any write. They reject missing/stale cut sources, a paste into the same parent, folder moves into themselves or descendants, collisions between selected roots, and case-insensitive sibling name conflicts. Folder copy allocates new folder/file ids while preserving hierarchy, file bytes, MIME/type, format, and enabled flags. Folder move updates only each canonical root folder parent so descendants retain their ids and relationships.

`DbSession` gains focused transaction-style methods for copying an asset tree and moving either a file or folder. Each method validates first and commits its SQL/data-array changes as one local operation. The UI marks the project dirty only after success.

Alternative considered: call `insertResourceFolder` and `upsertResourceFile` incrementally from the panel. `upsert` can overwrite a conflict, and an exception midway through a subtree would leave a partial copy.

### Reconcile Phoenix after the local operation succeeds

The project database remains the source of truth. Copy writes each enabled destination file to Phoenix. Cut writes enabled destination files before deleting their old enabled paths; folder deletion uses the existing recursive scoped API when safe. Disabled files remain database-only. Phoenix failures are recorded in Events as discrepancies but do not roll back a successful local database mutation; the normal full synchronization path can repair the engine snapshot.

Alternative considered: mutate Phoenix first. A later database failure could leave Phoenix ahead of the saved project and make rollback more complicated.

### Keep native file export behind a capability boundary

The web implementation always writes `text/plain`. A clipboard adapter may add a native file-list representation in a future desktop host or on a platform that explicitly supports it, using the same captured bytes. Unsupported formats must not make Copy/Cut fail and must not replace the plain-text representation.

Alternative considered: synthesize arbitrary `ClipboardItem` blobs and claim they paste as files in Explorer. The web specification and current browsers do not guarantee that OS-level file-list behavior, so it would be unreliable and MIME-dependent.

### Represent pending Cut as observable clipboard state

The clipboard service exposes its canonical cut-root identities. The Pool renderer adds an `is-cut-pending` class to matching file/folder rows, styled with `opacity: 0.5`. This is presentation only: rows remain selectable and accessible, and descendants are not individually styled unless they are canonical cut roots.

A successful internal paste clears the cut snapshot. Cacablu also clears it when the project session changes, source validation fails, another in-app Copy/Cut is observed, or the system clipboard no longer contains the emitted path/token. Clipboard replacement is checked on trusted `copy`, `cut`, and `paste` events; on `clipboardchange` where supported; and when the document regains focus/visibility if clipboard read permission permits. Because browsers prohibit continuous clipboard surveillance, external changes while Cacablu is unfocused are reflected as soon as the application can observe them.

Alternative considered: use a timer to poll the clipboard. That creates repeated permission prompts/security failures and still cannot bypass browser foreground restrictions.

## Risks / Trade-offs

- [The system clipboard is replaced after Cacablu stores an application snapshot] → Associate the snapshot with its emitted text/custom token and reject internal paste when the incoming clipboard representation no longer matches.
- [Large folder copies duplicate substantial byte arrays in memory] → Snapshot immutable references where safe, clone bytes only when inserting, and expose progress/error reporting if profiling shows a need.
- [Phoenix fails after the local database mutation] → Record an Event, retain the dirty authoritative project, and let the established project synchronization reconcile it.
- [Case sensitivity differs between SQLite names and Windows paths] → Preflight sibling names case-insensitively.
- [Cut styling becomes stale after project switch or source deletion] → Bind cut snapshots to a project session and clear/invalidate them on session replacement or failed source revalidation.
- [An external application replaces the clipboard while Cacablu is unfocused] → Revalidate on focus/visibility and describe the UI guarantee as removal when the change becomes observable.
- [Clipboard permissions or insecure context prevent async writes] → Use the user-initiated clipboard event where possible, report the failure, and never claim a successful external text copy when it was not written.

## Migration Plan

1. Add pure path, snapshot, destination-planning, and conflict-validation helpers with unit coverage.
2. Extend `DbSession` with atomic file/folder copy and move operations and database regression tests.
3. Extend Pool selection for multiple roots and add application clipboard state plus context-aware shell command routing.
4. Connect the Pool panel UI, 50%-opacity pending-cut state, clipboard replacement detection, and Phoenix reconciliation.
5. Verify keyboard/menu operation, Monaco text paste, recursive folders, project switching, conflicts, and disconnected Phoenix behavior.

Rollback removes the clipboard command handler/service and the new `DbSession` methods; no schema or stored-project migration is required.

## Open Questions

None. Native file-manager paste is explicitly capability-dependent and is not a release requirement for the browser build.
