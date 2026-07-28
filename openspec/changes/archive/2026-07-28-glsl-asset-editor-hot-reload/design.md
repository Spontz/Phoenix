## Overview

Phoenix will support two asset update modes for editor-driven workflows:

- **Preview mode**: Cacablu sends asset bytes to Phoenix for runtime use only. Phoenix updates an in-memory override, reloads affected sections, and does not touch disk.
- **Commit mode**: Cacablu writes the asset through the existing persisted asset operation. Phoenix writes disk, clears any preview override for that path, and reloads affected sections.

The design applies to GLSL files first, but the impact model is asset-based instead of shader-only so future texture, model, or script edits use the same section dependency behavior.

## API

### Transient Asset Preview

`PUT /api/assets/preview`

Request:

```json
{
  "requestId": "asset-preview-123",
  "path": "pool/shaders/example.glsl",
  "encoding": "utf-8",
  "content": "void main() {}"
}
```

`path` must be normalized relative to Phoenix `data` and must start with `pool/` or `resources/`. `encoding` may be `utf-8` for text assets or `base64` for binary-capable future assets.

Response:

```json
{
  "requestId": "asset-preview-123",
  "ok": true,
  "operation": "preview-asset",
  "path": "pool/shaders/example.glsl",
  "persisted": false,
  "reloadedSections": [{ "id": 17, "type": "efxBloom" }],
  "deactivatedSections": [],
  "failedSections": []
}
```

### Persisted Asset Operations

Existing persisted asset write, delete, unpublish, and move operations must return the same impact shape. Persisted writes set `persisted: true` and invalidate any preview override for the same path.

## In-Memory Asset Overlay

Phoenix maintains a runtime overlay map keyed by normalized asset path. Asset loaders consult the overlay before disk for paths under `pool` and `resources`.

Preview updates replace the overlay entry for the path. Persisted writes and deletes remove the overlay entry so disk becomes the source of truth again. The overlay is process-local and is cleared when Phoenix exits or loads a different project data folder.

The overlay must not affect `config` files or section `.spo` files.

## Section Dependency Index

Phoenix builds an index of loaded section dependencies:

- Section ID to normalized asset paths.
- Asset path to section IDs.

The index is refreshed when sections are loaded, replaced, updated, or deleted. Dependency extraction should prefer explicit section/resource metadata when available. If the current section parser does not expose structured dependencies yet, Phoenix may scan the accepted `.spo` content for exact normalized `pool/` and `resources/` references, but it must not invent aliases or accept unrelated fallback names.

## Reload And Deactivate Behavior

When an asset is previewed or committed, Phoenix uses the inverse dependency index to find every loaded section that references that exact normalized asset path. It reloads all of them on the engine thread using the current asset resolver before completing the request. Each section receives an independent result: successful reloads appear in `reloadedSections`, while failures remain inactive and appear in `failedSections`. One failure must not prevent Phoenix from attempting the remaining dependent sections.

A failed load is removed from the runtime but remains present in the dependency index so a later shader repair can retry it. Therefore the next reload treats an already-absent runtime section as an expected starting state instead of calling the diagnostic-producing removal path. It then loads the repaired script normally and reports the result.

When an asset is deleted, unpublished, or moved away from a path that sections still reference, Phoenix deactivates those dependent sections and reports them in `deactivatedSections`. Deactivated sections must not execute until a later asset restore, section update, or full section sync successfully reloads them.

All responses include exact section IDs so Cacablu can mark timeline bars and write Events.

## Cacablu Event Reporting

Immediately before a connected GLSL Preview or Phoenix write, Cacablu primes its existing recent-log cursor. Once the synchronous asset operation completes or fails, Cacablu reads the new Phoenix log entries. This captures the detailed compiler or linker message that `Shader::compile` writes to Phoenix's logger without replaying errors that predate the editor action.

Cacablu also converts every structured `failedSections` entry into an error Event and every `deactivatedSections` entry into a warning Event, preserving the section ID as the Event subject. The structured impact is the deterministic fallback and marks affected timeline sections; the correlated Phoenix logs retain the detailed GLSL diagnostics. Existing Events notification behavior opens the panel in the background and marks it unread for new errors.

Before applying new failures, Cacablu clears Timeline error IDs and resolved Phoenix Events for sections returned in `reloadedSections`, excluding any defensive overlap with failed or deactivated results. Phoenix log Events use an explicit source so this cleanup remains scoped and does not delete unrelated editor errors.

This approach deliberately reuses `/api/logs/recent` and the current asset-impact response instead of adding a second diagnostic protocol to Phoenix. A future API revision may add structured compiler diagnostics directly to the asset response if per-request diagnostic correlation becomes necessary.

## Error Handling

Phoenix returns `ok: false` with structured codes when the asset request itself cannot be applied:

- `invalid-asset-path`: path escapes `data`, targets `config`, or is not under `pool`/`resources`.
- `invalid-asset-content`: payload encoding or size is invalid.
- `asset-preview-failed`: overlay update or runtime reload failed before a stable response could be produced.
- `asset-write-failed`: persisted write/delete failed.

A stable asset preview or write remains `ok: true` when only some dependent sections fail to reload. Phoenix attempts every dependent section and returns those partial failures through `failedSections`, allowing Cacablu to report all outcomes from one response.

Phoenix must never show UI alerts for these API operations.

Cacablu must still record newly produced Phoenix errors when an asset request rejects, and it must not replay log entries that existed before that operation.

## Open Questions

- Which Phoenix loader path currently owns GLSL compilation and can most cleanly read from the overlay?
- Should future binary asset previews be enabled immediately, or should the first implementation restrict preview mode to text assets while preserving the base64 contract?
