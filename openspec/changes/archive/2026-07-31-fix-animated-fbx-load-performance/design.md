## Context

`planets.fbx` contains 254 meshes, and 252 of them use the same 4096x2048 `earth.png` texture. Phoenix currently constructs a `Material` inside every `Mesh` constructor. In slave mode, `TextureManager::m_forceLoad` makes each of those calls decode and upload the texture again.

The existing log records 251 forced reloads after the first load. At 4096x2048 RGBA, those repeated decodes process approximately 8,032 MiB, matching the reported transient memory use. A standalone Assimp import of the same file completes in approximately 0.08 seconds, so changing Assimp post-process flags would not address the root cause.

The mesh conversion also copies its completed vertex and index vectors into `Mesh`, creating avoidable temporary payload copies.

## Goals / Non-Goals

**Goals:**

- Load each Assimp material once per model.
- Prevent repeated texture decoding/upload when many meshes share a material.
- Preserve existing mesh-local material access and rendering.
- Move completed geometry containers into `Mesh`.
- Keep the implementation direct and readable.

**Non-Goals:**

- Change Assimp import flags.
- Change global forced-reload behavior for independently requested assets.
- Add benchmark, profiling, import-profile, or cache infrastructure.
- Redesign material, texture, model-cache, or animation systems.

## Decisions

### 1. Build a model-level material table

After Assimp succeeds and the model path is known, `Model::Load()` creates one `Material` for every `aiMaterial`. `processMesh()` selects the already loaded entry by `mMaterialIndex`.

Each `Mesh` keeps its current value-owned `Material`. Copying this small object preserves existing mesh-local access while its texture payload remains shared through `SP_Texture`. This avoids a wider ownership refactor.

Alternative considered: change `TextureManager::m_forceLoad`. Rejected because forced reload remains useful when an independently requested asset has changed; the bug is repeated construction of the same material within one model load.

Alternative considered: store shared material pointers in every mesh. Rejected because value copying is small, already matches the current API, and requires fewer changes.

### 2. Move geometry containers

`processMesh()` transfers completed vertex and index vectors to the `Mesh` constructor, which moves them into their final members. CPU-side geometry remains available to existing consumers.

### 3. Keep investigation artifacts temporary

The standalone Assimp probe and local measurements are not part of the production source and are removed before completion.

## Risks / Trade-offs

- **[A material is unexpectedly modified per mesh]** -> Preserve value-owned `Material` objects, so later mesh-local changes remain isolated.
- **[Material index is invalid]** -> Assimp validation and the existing scene contract continue to guarantee that `mMaterialIndex` addresses the scene material table.
- **[Load order changes texture creation timing]** -> Materials are still loaded after `ReadFile()` and before meshes are returned; only duplicate work is removed.

## Migration Plan

1. Load the Assimp material table once in `Model::Load()`.
2. Pass the selected loaded material to each mesh.
3. Move vertex and index containers into mesh ownership.
4. Remove temporary investigation artifacts.
5. Build and validate `planets.fbx` plus available model regressions.

Rollback consists of restoring per-mesh material loading and vector copying.

## Open Questions

None.
