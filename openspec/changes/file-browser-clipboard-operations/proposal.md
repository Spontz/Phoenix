## Why

The Pool file browser currently supports selection, drag import, drag move, and deletion, but its selected files and folders do not participate in normal Cut, Copy, and Paste workflows. Authors need to reorganize project assets quickly and paste stable `/pool/...` references directly into scripts and text fields without manually reconstructing paths.

## What Changes

- Route Cut, Copy, and Paste according to the current context: text controls retain native editing behavior, while the focused Pool browser operates on its selected assets.
- Support single and multiple Pool selection for clipboard operations and copy the selected files/folders into an application clipboard snapshot.
- Publish normalized `/pool/...` paths as `text/plain` on the system clipboard, using one path per line for multiple selected roots.
- Cut selected Pool items as pending internal moves and render their rows at 50% opacity until paste, invalidation, or an observed clipboard replacement; pasting their textual paths outside the Pool browser does not delete or move the sources.
- Paste copied items recursively into a selected Pool folder, or move cut items there, while preserving file contents and enabled state.
- Expose the Pool root as an explicit browser destination for internal paste and file drag-and-drop.
- Keep pasted asset changes in the project database and synchronize affected enabled paths with Phoenix using the existing asset operations.
- Reject stale sources, descendant folder moves, and destination name conflicts without partially mutating the project.
- Preserve ordinary text paste so Monaco, inputs, textareas, and contenteditable fields receive `/pool/...` paths automatically.
- Publish the same `/pool/...` text while dragging a Pool file so dropping it into a code editor inserts its path without moving the project asset.
- Treat native file-list export to Windows Explorer/Desktop as optional platform integration rather than a guaranteed web capability; the browser implementation guarantees internal asset paste plus `text/plain` paths.

## Capabilities

### New Capabilities

- `file-browser-clipboard`: Context-aware Cut, Copy, and Paste for Pool files and folders, including textual `/pool/...` clipboard representations and safe internal copy/move semantics.

### Modified Capabilities

None.

## Impact

- Pool browser selection, keyboard handling, paste targets, and pending-cut presentation in `src/panels/resources-panel.ts`.
- Application clipboard routing currently handled by `src/app/shell.ts` and Edit menu enablement/actions.
- Shared clipboard snapshot/path helpers and focused unit tests.
- `DbSession` resource mutation APIs for recursive folder copies and folder moves.
- Phoenix asset writes/deletes and Events reporting for copied or moved enabled assets.
- No Phoenix API or C++ changes are expected.
- Native operating-system file paste remains dependent on a future desktop/native host because web clipboard implementations do not guarantee a filesystem file-list representation.
