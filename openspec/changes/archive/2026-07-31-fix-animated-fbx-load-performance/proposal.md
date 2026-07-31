## Why

Loading the provided 2.7 MB animated `planets.fbx` through `drawScene` takes more than one minute and temporarily raises process memory by approximately 8 GB. Investigation shows that Assimp imports the file quickly, but Phoenix reloads the same 4096x2048 texture once for almost every mesh when forced asset reload is enabled.

## What Changes

- Load each Assimp material once per model instead of rebuilding it for every mesh that references it.
- Reuse the resulting material data across meshes while preserving existing material rendering behavior.
- Prevent avoidable CPU-side geometry copies during conversion from Assimp meshes to Phoenix meshes.
- Keep the implementation focused and readable, avoiding new profiling frameworks, permanent benchmark code, or unnecessary import abstractions.
- Validate the result manually with `planets.fbx` and existing `drawScene` behavior.

## Capabilities

### New Capabilities

- `animated-model-load-performance`: Efficient loading of animated models through `drawScene`, with correct animation and rendering output.

### Modified Capabilities

None.

## Impact

- Model import and conversion in `Engine/src/core/renderer/Model.cpp`, `Mesh.cpp`, and their headers.
- Material creation during model conversion; no dependency upgrade is required.
- Manual validation using `Launcher_WD/data/pool/models/planets/planets.fbx`.
