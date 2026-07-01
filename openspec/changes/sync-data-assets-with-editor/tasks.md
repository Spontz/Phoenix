## 1. Phoenix Asset Model

- [x] 1.1 Add a Phoenix asset path helper that normalizes relative paths, rejects absolute/path-traversal inputs, and allow-lists only `pool` and `resources`.
- [x] 1.2 Add manifest generation for `DEMO->m_dataFolder` covering files and directories under `pool` and `resources`.
- [x] 1.3 Include relative path, entry type, file size, and content hash for manifest file entries.
- [ ] 1.4 Add unit-testable coverage or focused checks for valid paths, rejected `config` paths, rejected traversal, and Windows separator normalization.
- [x] 1.5 Resolve the repository debug-build default data root to `Launcher/data` when no `-datafolder` override is provided.

## 2. Phoenix Editor API

- [x] 2.1 Add a browser-compatible endpoint or WebSocket request for retrieving Phoenix's asset manifest.
- [x] 2.2 Add asset operation handling for create directory, write file, delete file, and delete directory.
- [ ] 2.3 Implement safe file writes using a temporary file followed by replacement.
- [x] 2.4 Return structured success and error payloads with stable codes for all asset operations.
- [ ] 2.5 Publish asset change events over the existing editor WebSocket after successful mutations.
- [ ] 2.6 Keep the editor API active only in Phoenix slave mode, preserving standalone behavior.
- [x] 2.7 Return browser-compatible CORS and Private Network Access headers without duplicated `Access-Control-Allow-Origin` values.

## 3. Cacablu Folder Sync

- [x] 3.1 Gate asset sync behind Cacablu having a loaded project.
- [x] 3.2 Collect enabled pool files from the loaded project database as the expected published pool manifest.
- [x] 3.3 Request Phoenix's active engine manifest and compare Phoenix `pool` against the expected published pool.
- [x] 3.4 Show blocking progress while initial project pool sync runs.
- [x] 3.5 Clean Phoenix `pool` and upload all enabled pool files when the manifest is not an exact match.
- [x] 3.6 Allow cancelling initial sync and keep Cacablu without a loaded project/pool after cancellation.

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
- [ ] 5.6 Verify `config`, absolute paths, and traversal paths are rejected without modifying files.
- [x] 5.7 Verify existing runtime state, input, WebRTC signaling, and Phoenix standalone startup behavior still work.
