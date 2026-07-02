## 1. Phoenix Asset Model

- [x] 1.1 Add a Phoenix asset path helper that normalizes relative paths, rejects absolute/path-traversal inputs, and allow-lists only `pool` and `resources`.
- [x] 1.2 Add manifest generation for `DEMO->m_dataFolder` covering files and directories under `pool` and `resources`.
- [x] 1.3 Include relative path, entry type, file size, and content hash for manifest file entries.
- [x] 1.4 Add unit-testable coverage or focused checks for valid paths, rejected `config` paths, rejected traversal, and Windows separator normalization.
- [x] 1.5 Resolve the repository debug-build default data root to `Launcher/data` when no `-datafolder` override is provided.

## 2. Phoenix Editor API

- [x] 2.1 Add a browser-compatible endpoint or WebSocket request for retrieving Phoenix's asset manifest.
- [x] 2.2 Add asset operation handling for create directory, write file, delete file, and delete directory.
- [x] 2.3 Implement safe file writes using a temporary file followed by replacement.
- [x] 2.4 Return structured success and error payloads with stable codes for all asset operations.
- [x] 2.5 Publish asset change events over the existing editor WebSocket after successful mutations.
- [x] 2.6 Keep the editor API active only in Phoenix slave mode, preserving standalone behavior.
- [x] 2.7 Return browser-compatible CORS and Private Network Access headers without duplicated `Access-Control-Allow-Origin` values.

## 3. Cacablu Folder Sync

- [x] 3.1 Gate asset sync behind Cacablu having a loaded project.
- [x] 3.2 Collect enabled pool files from the loaded project database as the expected published pool manifest.
- [x] 3.3 Request Phoenix's active engine manifest and compare Phoenix `pool` against the expected published pool.
- [x] 3.4 Show blocking progress while initial project pool sync runs.
- [x] 3.5 Clean Phoenix `pool` and upload all enabled pool files when the manifest is not an exact match.
- [x] 3.6 Allow cancelling initial sync and keep Cacablu without a loaded project/pool after cancellation.

## 3A. Phoenix Section Model And API

- [x] 3A.1 Add a Phoenix section manifest serializer for current runtime sections.
- [x] 3A.2 Include stable section id, type, start/end times, enabled state, layer, blend factors, blend equation, and canonical script content or script hash in the manifest.
- [x] 3A.3 Add an editor API endpoint or WebSocket request for retrieving the section manifest.
- [x] 3A.4 Add an editor API endpoint or WebSocket request for full section replacement from a Cacablu bar snapshot.
- [x] 3A.5 Reuse `SectionEventManager`, `DemoKernel::loadScriptFromNetwork`, `SectionManager`, or a shared helper around those paths for section creation/deletion where practical.
- [x] 3A.6 Make section replacement all-or-nothing where possible, returning a structured error without leaving partially replaced sections on validation or parse failure.
- [x] 3A.7 Rebuild section load/execution queues after successful replacement so playback reflects the new bars.
- [x] 3A.8 Publish a structured section change event over the editor WebSocket after successful replacement.
- [x] 3A.9 Persist every received section as `<id>.spo` directly under Phoenix's active `data` folder.
- [x] 3A.10 Format persisted section files with `:::<type>`, `id`, `start`, `end`, `enabled`, `layer`, `blend`, `blendequation`, one blank line, and the raw script body.
- [x] 3A.11 Delete the corresponding root `<id>.spo` file whenever an editor-published runtime section is deleted.
- [x] 3A.12 Remove stale editor-published root section `.spo` files during full replacement so disk state matches the received section set.

## 3B. Cacablu Bar Sync

- [x] 3B.1 Serialize project database `bars` into canonical Phoenix section payloads equivalent to root `.spo` section files.
- [x] 3B.2 Request Phoenix's section manifest during project-open sync after the initial pool sync is complete or skipped.
- [x] 3B.3 Compare serialized bars with Phoenix sections by id, type, start/end, enabled state, layer, blend metadata, and script content/hash.
- [x] 3B.4 Skip section replacement when the serialized project bars exactly match Phoenix's section manifest.
- [x] 3B.5 Send a full section replacement request when any project bar or Phoenix section is missing, extra, or changed.
- [x] 3B.6 Show section sync progress in the same blocking project-open modal and allow Cancel to leave Cacablu without the opened project loaded.
- [x] 3B.7 Surface Phoenix section replacement errors clearly and keep Cacablu usable.

## 4. Cacablu Asset Operations

- [x] 4.1 Prevent file transfer actions when no Cacablu project is loaded.
- [x] 4.2 Send file create and replace operations to Phoenix when published pool files are imported, moved, or enabled.
- [x] 4.3 Send delete file operations to Phoenix when published pool files are deleted or disabled.
- [x] 4.4 Send create/delete directory operations to Phoenix for directory changes under allowed project asset paths.
- [x] 4.5 Prevent or warn on attempts to sync `config` or paths outside `pool` and `resources`.
- [x] 4.6 Refresh local/engine discrepancy state after Phoenix confirms or rejects an operation.

## 5. Validation

- [x] 5.1 Verify Phoenix manifests and writes are generated from repository `Launcher/data` in the debug build default.
- [ ] 5.2 Verify Phoenix manifests are generated from a `-datafolder` override.
- [x] 5.3 Verify Cacablu detects exact-match versus non-matching published pool manifests.
- [x] 5.4 Verify Cacablu does not compare or transfer assets when no project is loaded.
- [x] 5.5 Verify create, replace, delete file, create directory, and delete directory operations affect only Phoenix `data/pool` and `data/resources`.
- [x] 5.6 Verify `config`, absolute paths, and traversal paths are rejected without modifying files.
- [x] 5.7 Verify existing runtime state, input, WebRTC signaling, and Phoenix standalone startup behavior still work.
- [x] 5.8 Verify exact-match project bars skip Phoenix section replacement.
- [x] 5.9 Verify changed project bars cause Phoenix sections to be deleted and recreated from the Cacablu snapshot.
- [ ] 5.10 Verify invalid section replacement requests are rejected without partially replacing Phoenix sections.
- [ ] 5.11 Verify section replacement writes `<id>.spo` files directly under the active Phoenix `data` folder with the expected contents.
- [ ] 5.12 Verify deleting sections also deletes their corresponding root `<id>.spo` files from the active Phoenix `data` folder.
