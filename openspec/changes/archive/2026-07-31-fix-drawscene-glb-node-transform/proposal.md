## Why

`drawScene` currently renders multi-object GLB files (for example Blender exports) with all objects collapsed at the origin. This breaks expected scene composition and makes object-level placement authored in DCC tools invisible at runtime.

The root cause is that Phoenix loads meshes from Assimp nodes but does not preserve/apply per-node transforms when rendering static models in `drawScene`.

## What Changes

- Preserve per-node transforms from Assimp during model import and rendering for static `drawScene` path.
- Compose each mesh runtime model matrix as:
  - `sectionBaseModel * nodeGlobalTransform` for static rendering.
- Keep section expression transform (`tx/ty/tz/rx/ry/rz/sx/sy/sz`) as a global model transform that affects the whole imported scene.
- Ensure repeated node references to the same Assimp mesh index render as distinct instances when their node transforms differ.
- Add debug visibility to inspect node/mesh transform mapping.

## Scope

### In Scope

- `drawScene` + `Model`/`Mesh` import-render path for static (non-skeletal) placement.
- Node hierarchy transform accumulation and runtime application.
- Regression validation using the reported `cubes.glb` case.

### Out of Scope

- Changing section script contract for `drawScene`.
- Changing shader interface beyond existing `model/view/projection` usage.
- Reworking skeletal animation architecture.

## Impact

Expected touched areas (implementation phase):

- `Engine/src/core/renderer/Model.h`
- `Engine/src/core/renderer/Model.cpp`
- `Engine/src/core/renderer/Mesh.h`
- `Engine/src/core/renderer/Mesh.cpp`
- `Engine/src/sections/drawScene.cpp` (only if needed for debug plumbing)

No code changes are made in this proposal step.
