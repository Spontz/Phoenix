## Why

Cacablu and Phoenix need to operate against the same demo `data` tree and timeline while the editor is open, but today Phoenix only reads its local `data` folder at startup and exposes no focused way for the browser editor to compare or push asset or section changes. This change gives Cacablu a controlled synchronization path for published `pool` assets and project bars so editor-side file and timeline state can be mirrored into the running engine.

## What Changes

- Add a Phoenix editor API asset manifest for the engine's active `data/pool` and `data/resources` trees.
- Add a Cacablu-to-Phoenix synchronization handshake when Cacablu opens a project. Cacablu treats the project's published database assets as authoritative and no longer asks the user to select Phoenix's `data` folder.
- Add a Phoenix editor API section manifest and replace operation so Cacablu can publish project database `bars` as Phoenix runtime sections.
- During project-open sync, compare Cacablu's serialized bars with Phoenix's current sections. If they match exactly, skip section mutation; if not, delete Phoenix runtime sections and recreate them from the project's bars.
- Add safe file create, replace, delete, and directory operations for paths under `pool` and `resources`.
- Require a loaded Cacablu project before any file transfer to Phoenix can run.
- Restrict this initial asset sync scope to `pool` and `resources`; `config` remains read-only/out of scope for this change.
- Publish asset change results over the existing editor WebSocket so Cacablu can update UI state after Phoenix applies or rejects an operation.
- Preserve `-datafolder` overrides, and in the repository debug build resolve the default engine data root to `Launcher/data` so Cacablu writes to the same tree Phoenix uses during development.

## Capabilities

### New Capabilities
- `data-asset-sync`: Covers manifest comparison, discrepancy reporting, and controlled mutation of `data/pool` and `data/resources` from Cacablu to Phoenix.
- `section-bar-sync`: Covers project `bars` to Phoenix `sections` snapshot comparison and full replacement through the editor API.

### Modified Capabilities

## Impact

- Phoenix editor API: new HTTP endpoints and/or WebSocket messages for asset manifests and asset mutations.
- Phoenix section runtime: new editor API endpoints/messages for section manifest retrieval and full section replacement, reusing the existing section event/loading behavior where possible.
- Phoenix filesystem handling: path normalization, subtree allow-listing, safe writes, directory creation, and deletion under the active `data` folder.
- Cacablu editor: when a project is loaded, publish enabled `pool` assets and serialized bars from the project database to Phoenix, compare against Phoenix's manifests, clean stale Phoenix pool contents and replace Phoenix sections when the published state differs, and mirror subsequent asset operations to Phoenix.
- Runtime behavior: Phoenix continues loading `config/*.spo` and root `.spo` files as today; this change does not reload the whole demo automatically unless a later implementation explicitly adds targeted hot-reload behavior.
