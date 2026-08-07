## Why

`Model::Draw` currently mixes per-frame *computation* (mesh transform composition, skeletal animation, animated-camera resolution) with *rendering* (uniform uploads and draw calls). The animated cameras embedded in a model file are only resolved deep inside the bone hierarchy walk (`ReadNodeHeirarchy`), which runs inside `Draw` — so by the time a section could read that camera, the geometry has already been submitted. Sections evaluate their script expression *before* `Draw`, which makes it impossible today to drive `tx/ty/tz/rx/ry/rz/sx/sy/sz` or any shader variable from the model's own animated camera.

The same coupling also makes the animation cost scale with the number of draws: `drawSceneMatrix` and `drawSceneMatrixFolder` call `Draw` once per instance, so a fully animated model re-walks its entire node hierarchy once per instance, per frame, even though the result is identical for every instance.

## What Changes

- Introduce an explicit **per-frame precalculation phase** on `Model`, separate from rendering. It resolves, once per frame per model: the selected animation's node hierarchy, the bone transformation set, the animated node transform of every mesh, and the view matrix of every camera contained in the model.
- Reduce `Model::Draw` to the rendering phase: compose the final mesh model matrices from the section-supplied base matrix and the precalculated animated node transform, upload uniforms (including the already-computed bone transforms), issue draw calls, and roll over the previous-frame matrices used by motion blur.
- Stop accumulating animation results into `Mesh::m_matModel`. The animated node transform is stored in its own field so that the precalculation result is independent of the per-instance base model matrix, and so the precalculation is idempotent and reusable across draws.
- Apply the same split to `ModelInstance`: a precalculation entry point that delegates to the underlying `Model` and refreshes the instance matrix buffer, and a `drawInstanced` that only binds and issues the instanced draw.
- Expose the model's resolved camera (position, target/front, up vector, orientation angles, field of view and view matrix) after precalculation, so a section can publish it to its expression evaluator before evaluating the formula that positions the model and feeds shader variables.
- Update the five sections that render models so their per-frame order becomes: evaluate expression → precalculate → publish model camera → re-evaluate expression → set matrices and shader variables → draw.
- Remove the `setBoneTransformations` visibility hack on `Model` (documented in the header as `// Hack for ModelInstance:: TODO: Make it private again`), which exists only because `ModelInstance` needed to trigger the computation stage from outside.

**BREAKING** (engine-internal only): `Model::Draw` and `ModelInstance::drawInstanced` no longer perform animation computation on their own. Any caller must invoke the precalculation phase first. All in-tree callers are updated by this change; no demo script, `.spo` file or shader is affected.

## Capabilities

### New Capabilities
- `model-frame-precalc`: Defines the two-phase per-frame contract for rendering models — a computation phase that resolves animation, mesh transforms and model cameras, and a rendering phase that only submits GPU work — together with the rules for how sections order these phases around expression evaluation, how the model's animated camera becomes available to script expressions, and the invariants that guarantee identical rendering output to the current single-phase behavior.

### Modified Capabilities
<!-- None. `drawscene-glb-node-transform-fix` and `model-animation-interpolation` constrain *what* transforms and poses are produced; this change only relocates *when* they are computed, and both specs must continue to hold unchanged. -->

## Impact

Engine code:
- `Engine/src/core/renderer/Model.h` / `Model.cpp` — new precalculation entry point, reduced `Draw`, animated node transform stored separately, camera accessors, `setBoneTransformations` made private again.
- `Engine/src/core/renderer/Mesh.h` — new field holding the animated node transform, kept distinct from `m_matModel` and `m_matNodeGlobal`.
- `Engine/src/core/renderer/ModelInstance.h` / `ModelInstance.cpp` — precalculation entry point, `drawInstanced` reduced to GPU submission.
- `Engine/src/core/renderer/Camera.h` / `CameraTypes.cpp` — populate the descriptive camera state (position, front, up, angles) for cameras defined by a raw view matrix, which currently expose default values only.
- `Engine/src/core/drivers/MathDriver.h` / `MathDriver.cpp` — expression variables for the active model camera.

Sections that call the model render path and must adopt the new order:
- `Engine/src/sections/drawScene.cpp`
- `Engine/src/sections/drawSceneMatrix.cpp`
- `Engine/src/sections/drawSceneMatrixFolder.cpp`
- `Engine/src/sections/drawSceneMatrixInstanced.cpp`
- `Engine/src/sections/drawSceneMatrixInstancedFolder.cpp`

Not affected: particle, emitter, FBO, video, image, volume and effect sections, which never call `Model::Draw`.

Performance: animated models drawn N times per frame drop from N node-hierarchy walks to one. No expected regression for single-draw, non-animated models.
