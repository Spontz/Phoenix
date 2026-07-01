## Context

Phoenix currently resolves its demo data from `DemoKernel::m_dataFolder` and accepts an override through `-datafolder`. In the repository debug build, the default development data root resolves to `Launcher/data` so the editor API writes to the same folder the user inspects while developing. If that repository folder is not present, Phoenix falls back to a `data` folder next to the executable. Startup verifies the folder exists, loads `config/*.spo`, then loads root `.spo` files, and most runtime resources are referenced by concatenating `m_dataFolder` with relative paths under `pool` or `resources`.

The native editor API already runs only in slave mode and exposes browser-compatible HTTP/WebSocket plumbing for Cacablu. It currently has health, runtime/input, and WebRTC signaling behavior, but no focused file synchronization contract. Cacablu needs to open a local project, use that project's database-backed asset state as the authority for published `pool` files, compare it with Phoenix's active `data` folder, and send selected file operations so Phoenix can mirror changes into its own `data/pool` and `data/resources` trees.

## Goals / Non-Goals

**Goals:**
- Let Cacablu and Phoenix confirm whether Phoenix's `data/pool` matches the loaded project's published pool assets.
- Require a loaded Cacablu project before any asset transfer to Phoenix can start.
- Let Cacablu create, replace, delete, and create directories in Phoenix's active `data/pool` and `data/resources` folders.
- Clean Phoenix's `data/pool` before the initial project sync unless the published pool manifest already matches exactly.
- Keep all Phoenix writes constrained to the active `data` folder and only the allowed subtrees.
- Reuse the existing editor API server and WebSocket event path.

**Non-Goals:**
- No `config` synchronization in this change.
- No automatic full demo reload after asset mutation.
- No conflict-resolution UI beyond clear discrepancy reporting and explicit user/editor action.
- No general-purpose filesystem API outside `data/pool` and `data/resources`.

## Decisions

### Gate transfers on a loaded Cacablu project

Cacablu only starts manifest comparison and file transfer when an editor project is loaded. Opening or selecting a raw folder without an active project does not send asset mutations to Phoenix.

Rationale: asset transfer needs project context so Cacablu knows which `data` tree is authoritative for the current editing session and can present discrepancies in the right project UI.

Alternative considered: allow ad hoc folder transfer without a project. This is flexible, but it makes it too easy to push files from the wrong folder into the running engine.

### Use manifest comparison as the opening handshake

Cacablu collects enabled pool files from the loaded project database and compares those published file paths and sizes with Phoenix's active engine manifest. Phoenix exposes its own manifest for the active engine `data` folder using normalized forward-slash relative paths rooted at `data`, file size, and content hash for files. Directories are represented as entries with type `directory`.

Rationale: the database `enabled` state defines which pool files are published to Phoenix. Comparing the expected published file set before upload lets Cacablu skip an exact match, while avoiding stale files from a previous project.

Alternative considered: send all files immediately on project open. This is simpler, but it is wasteful when Phoenix already contains the exact published pool and can leave stale files unless the pool is cleaned.

### Clean Phoenix pool before non-matching initial sync

On project open, Cacablu compares Phoenix's `pool` file manifest with the project's enabled pool files. If the file set and sizes match exactly, Cacablu skips copying. If there is any missing, extra, or size-mismatched file, Cacablu deletes Phoenix's `pool` directory recursively, recreates it, and uploads every enabled pool file. The operation is shown as a blocking modal with progress and a Cancel action. If cancelled, Cacablu does not publish the newly opened project session and the editor remains without a loaded pool.

Rationale: a strict clean sync prevents stale assets from a previous project from remaining in Phoenix while still avoiding needless copies when the manifest is already exact.

### Keep mutations explicit and operation based

Cacablu sends individual asset operations: create directory, write file, delete file, and delete directory. File writes include bytes and metadata for one path. Phoenix validates and applies each operation, then emits an asset event indicating success or failure.

Rationale: explicit operations map cleanly to editor actions and make errors local to one path. They also avoid requiring Phoenix to implement a large batch sync transaction before the first useful version.

Alternative considered: batch upload an entire subtree. This is useful later for "sync all", but it makes partial failure and progress reporting harder.

### Restrict paths with a normalized subtree allow-list

Phoenix resolves every requested path against `DEMO->m_dataFolder`, rejects absolute paths, rejects `..`, rejects path traversal after canonicalization, and accepts only paths whose first segment is `pool` or `resources`.

Rationale: the editor API is reachable from a browser and performs writes. The implementation must be safe even if a malformed or malicious client sends paths such as `../config/control.spo` or absolute Windows paths.

Alternative considered: trust Cacablu to send valid paths. This is not acceptable for a network-facing write API.

### Use safe file writes for replacements

Phoenix writes file content to a temporary file in the destination directory and atomically replaces the target when possible. Parent directories are created only inside allowed subtrees.

Rationale: shader, texture, model, and other resource writes should not leave half-written files if Cacablu disconnects or the write fails.

Alternative considered: direct overwrite. It is easier but can corrupt files used by the running engine.

### Do not hot-reload assets by default

Phoenix records and emits asset changes, but this change does not require immediate resource-manager invalidation or shader/texture/model reload. Any later hot-reload behavior can build on the same events.

Rationale: Phoenix currently loads many resources during script and section loading. Automatic reload has separate lifetime and GPU-resource risks that should be designed independently.

## Risks / Trade-offs

- Large files can be expensive to hash and transfer -> Bound request sizes where practical, stream or chunk later if large assets become common, and keep the first implementation focused on correctness.
- Engine and editor may point at different `data` folders by design -> Treat this as a warning/discrepancy, not as a hard connection failure.
- Deleting files while Phoenix has them loaded may not affect current GPU/audio/model objects -> Emit events and document that runtime reload is outside this change.
- Windows path casing and separators can cause false differences -> Normalize separators and compare paths case-insensitively on Windows while preserving display names.
- Directory deletion can remove many files -> Require recursive deletes to be explicit and still constrained to `pool` or `resources`.
- Browser private-network and CORS rules can block Cacablu-to-Phoenix requests -> Return a single CORS origin header and include `Access-Control-Allow-Private-Network: true` for browser preflights.

## Migration Plan

1. Add Phoenix-side manifest generation and validation helpers for the active `data` folder.
2. Add editor API routes/messages for manifest exchange and asset operations.
3. Add Cacablu project-open pool sync for enabled database assets.
4. Wire Cacablu file create/update/delete and enabled-checkbox actions to Phoenix asset operations.
5. Verify existing runtime, input, WebRTC, and standalone Phoenix behavior remains unchanged.
