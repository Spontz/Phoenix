## 1. Clipboard and Selection Model

- [x] 1.1 Extend shared Pool asset selection to represent ordered file/folder multi-selections while preserving current single-selection consumers.
- [x] 1.2 Add pure helpers for Ctrl/Cmd toggle selection, visible-tree Shift ranges, ancestor/descendant canonicalization, and deterministic visible-order roots.
- [x] 1.3 Add normalized `/pool/...` path formatting and newline-delimited plain-text serialization with traversal and duplicate-root protection.
- [x] 1.4 Implement an application asset clipboard service that stores copy snapshots or project-bound cut references, exposes pending cut roots, and notifies subscribers.
- [x] 1.5 Invalidate pending cuts on replacement, project-session change, stale sources, observable external clipboard changes, and successful internal paste.
- [x] 1.6 Add unit tests for selection, canonical roots, path text, copy persistence, cut consumption, and clipboard invalidation.

## 2. Atomic Database Asset Operations

- [x] 2.1 Add destination planning that resolves Pool root/folder/file-parent targets and rejects ambiguous multi-selection destinations.
- [x] 2.2 Add case-insensitive conflict, stale-source, same-parent, and folder self/descendant cycle validation before mutation.
- [x] 2.3 Extend `DbSession` with one atomic operation to recursively copy one or more canonical file/folder roots with new ids and preserved content, metadata, hierarchy, and enabled state.
- [x] 2.4 Extend `DbSession` with one atomic operation to move one or more canonical file/folder roots while preserving ids and descendant relationships.
- [x] 2.5 Ensure SQL and in-memory project data roll back together when a planned copy or move fails.
- [x] 2.6 Add database tests for files, nested folders, multiple roots, duplicate descendant selection, conflicts, cycles, rollback, and save/reopen persistence.

## 3. Command Routing and Pool UI

- [x] 3.1 Replace generic non-text clipboard shortcut handling with a shared Cut/Copy/Paste command route used by both keyboard shortcuts and Edit menu actions.
- [x] 3.2 Preserve native Cut/Copy/Paste in Monaco, inputs, textareas, selects, and contenteditable elements, with no Pool mutation.
- [x] 3.3 Implement Pool row multi-selection with Ctrl/Cmd toggle, Shift visible range, and a stable selection anchor.
- [x] 3.4 Make Pool Copy/Cut capture canonical roots and publish their `/pool/...` paths as `text/plain`, retaining internal paste when optional native/custom formats are unavailable.
- [x] 3.5 Render canonical pending-cut rows with an `is-cut-pending` state at exactly 50% opacity while keeping them interactive and accessible.
- [x] 3.6 Revalidate pending cut ownership on trusted clipboard events, supported `clipboardchange`, and permitted focus/visibility clipboard checks without polling.
- [x] 3.7 Implement Pool Paste into the resolved destination, retain copy snapshots for repeated paste, and clear cuts only after successful paste.
- [x] 3.8 Update Edit menu enablement and user-facing conflict/stale/ambiguous-destination feedback for the active clipboard context.
- [x] 3.9 Render an explicit Pool root destination and route root selection, internal file drops, external file drops, and Paste to parent id `0`.

## 4. Phoenix Reconciliation

- [x] 4.1 Collect old and new enabled file paths from each successful local copy/move result.
- [x] 4.2 Write enabled destination files through existing scoped Phoenix asset APIs and skip Phoenix operations while disconnected.
- [x] 4.3 For moves, attempt destination writes before deleting old enabled file/folder paths and avoid deleting overlapping destinations.
- [x] 4.4 Record Phoenix write/delete discrepancies in Events without rolling back the authoritative dirty project.
- [x] 4.5 Add tests for connected copy, connected cut, disabled assets, disconnected edits, partial Phoenix failure, and later full-sync compatibility.

## 5. Integration Verification

- [x] 5.1 Verify keyboard and Edit menu commands produce identical Pool behavior and do not interfere with existing timeline or text-editing shortcuts.
- [x] 5.2 Verify copied and cut paths paste into Monaco and ordinary text fields as normalized `/pool/...` text, one path per line.
- [x] 5.3 Verify pending-cut rows remain at 50% opacity until paste or observable clipboard replacement, including project switch and external replacement on focus.
- [x] 5.4 Verify recursive and multiple-root operations remain all-or-nothing and the SQLite project saves and reloads with the expected tree.
- [x] 5.5 Run Cacablu typecheck, unit/integration tests, lint, production build, and strict OpenSpec validation; confirm no Phoenix change is required. (Targeted lint passes; repository-wide lint remains blocked by three pre-existing errors outside this change.)
- [x] 5.6 Publish Pool path text synchronously during the native Copy/Cut gesture and verify the asynchronous fallback plus a real Monaco paste in Chromium.
- [x] 5.7 Route Ctrl/Cmd+C and Ctrl/Cmd+X through their original native clipboard events and verify a real keyboard copy followed by a Monaco paste.
- [x] 5.8 Publish a dragged Pool file as normalized plain text while preserving its private internal-move payload, and verify a real drop into Monaco.
- [x] 5.9 Verify copied and cut items paste into the Pool root and internal/external file drops resolve the root path without affecting folder targets.
