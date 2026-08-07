## Context

See proposal.md — Why. The constraints that shape the approach, as they exist in the code today:

- `Model::Draw` performs, in one call: `setMeshesModelTransform()`, `setBoneTransformations()` (which walks the node hierarchy *and* uploads `gBones`), the model-camera view override, all uniform uploads, all mesh draws, and the previous-matrix rollover for motion blur.
- `Model::ReadNodeHeirarchy` writes its results into three places: `meshes[i]->m_matModel *= globalInverse * GlobalTransformation` (an **accumulation** onto whatever `setMeshesModelTransform` just wrote), `m_camera[i]->setViewMatrix(inverse(GlobalTransformation))`, and `m_boneInfoMap[...].FinalTransformation`.
- Because of that accumulation, `Draw` is *not* idempotent: the animated node transform is entangled with `m_matBaseModel`, which the matrix sections change between draws. This is the single structural blocker for hoisting the computation out.
- `m_matBaseModel` is written per draw by `drawSceneMatrix` and `drawSceneMatrixFolder` inside their instance loops, so those sections currently trigger one full hierarchy walk per instance per frame.
- `Mesh` already carries `m_matModel`, `m_matPrevModel` and `m_matNodeGlobal`; there is no field for the animated node transform.
- Model cameras are `CameraRawMatrix`, which stores only `m_Matrix`. Its inherited `m_Position`, `m_Front`, `m_Up`, `m_Yaw`, `m_Pitch`, `m_Roll` are never populated, so they read as constructor defaults.
- `MathDriver` binds `cam_*` expression variables by reference to `CameraManager::m_pActiveCameraExprTk`, a copy refreshed only inside `setActiveCamera`. `MathDriver`'s symbol table is built once per section at construction time.
- `Section::warmExec()` is implemented as a plain call to `exec()` in all five affected sections, so any ordering fix in `exec` is automatically covered by warm-up.

## Goals / Non-Goals

**Goals:**

- A precalculation entry point whose result is a pure function of (model, selected animation, playback time) — independent of any per-draw base transform, and therefore safe to call once and consume many times.
- A render entry point that is pure GPU submission plus per-draw matrix composition.
- Make the model's animated camera readable by a section between the two phases, with usable descriptive state (position, front, up, angles, fov), not just a raw matrix.
- Byte-for-byte equivalent rendering for every existing demo, with the per-instance hierarchy walk eliminated as a side effect.

**Non-Goals:**

- Introducing an engine-wide render-graph or a global "update then render" pass over all sections. The phase ordering stays inside each section's `exec()`.
- Changing how animation keyframes are sampled or interpolated (owned by `model-animation-interpolation`) or how static node transforms compose (owned by `drawscene-glb-node-transform-fix`).
- Extracting bone handling into a dedicated class (the existing `// TODO: Do a Bones Class` stays a TODO).
- Caching precalculation across frames, or deduplicating precalculation between two sections that happen to share the same `SP_Model` in the same frame.
- Changing the `.spo` script format or any shader.

## Decisions

### D1 — Store the animated node transform in its own `Mesh` field

`Mesh` gains `m_matNodeAnimated`, written by the precalculation phase, defaulted to identity. `ReadNodeHeirarchy` **assigns** into it (`= globalInverse * GlobalTransformation`) instead of multiplying into `m_matModel`.

The render phase then computes, per mesh:

- animation enabled → `m_matModel = m_matBaseModel * m_matNodeAnimated`
- animation disabled → `m_matModel = m_matBaseModel * m_matNodeGlobal`

This is exactly the composition `setMeshesModelTransform` + `ReadNodeHeirarchy` produce today (`(m_matBaseModel) * (globalInverse * GlobalTransformation)`), so the output is unchanged, but the accumulation becomes an assignment and precalculation becomes idempotent.

*Alternative considered:* keep the accumulation and simply call `Draw` once per instance as today, hoisting only the camera resolution. Rejected — it leaves the per-instance recomputation in place, and the camera would still be resolved from a walk driven by the previous instance's base transform, which is fragile.

*Alternative considered:* have precalculation write the final `m_matModel` and require sections to set `m_matBaseModel` before precalculating. Rejected — it forces one precalculation per instance, defeating the main perf goal, and reintroduces the ordering trap.

### D2 — `Model::PreCalc(float currentTime)` and a slimmed `Model::Draw(shader, startTexUnit)`

`PreCalc` runs: animation sampling and node walk (when `playAnimation` and the scene has animations), producing `m_boneTransforms`, each mesh's `m_matNodeAnimated`, and each model camera's view matrix; then it resolves the model camera into `m_matModelCameraView` and a validity flag when `useCamera` is set and the index is in range.

`Draw` keeps its `currentTime` parameter **removed** from its signature. Leaving an ignored parameter in place would let a caller believe `Draw` still drives the animation, which is precisely the confusion this change removes. All five call sites are updated in the same change, so the signature break is contained.

`Draw` applies the model camera view override from the precalculated value, composes per-mesh model matrices per D1, uploads `gBones` (from the already-computed `m_boneTransforms`), uploads the matrix uniforms, draws, and performs the previous-matrix rollover exactly as today.

`setBoneTransformations` disappears as a public entry point: the computation moves into `PreCalc` and the `gBones` upload moves into `Draw`. This resolves the `// Hack for ModelInstance:: TODO: Make it private again` note in the header.

### D3 — `ModelInstance::PreCalc(float currentTime)` delegates the model precalculation

`ModelInstance::PreCalc` calls `m_pModel->PreCalc(currentTime)`. `drawInstanced` drops its `currentTime` parameter and becomes bind + `glDrawElementsInstanced` + material binding + the `gBones` upload.

The instance-matrix buffer upload stays owned by `updateMatrices()`. The sections already call it from inside their own `updateMatrices(bool)` helper, which runs *after* the expression that positions the instances and *before* `drawInstanced` — an ordering that already matches the new contract. Folding that upload into `PreCalc` as well would push the same buffer to the GPU twice per frame, and would upload stale instance matrices on the first of the two.

Note for the instanced path: when `playAnimation` is true, `ModelInstance::updateMatrices` deliberately skips `meshNodeTransform` and uploads the raw instance matrices. That asymmetry is pre-existing and is preserved as-is; folding `m_matNodeAnimated` into the instanced path is a separate concern.

Because `drawInstanced` never routes through `Model::Draw`, the `gBones` upload cannot live inside `Draw` alone. Both render paths call a shared `Model::uploadBoneTransforms(shader)` helper, which only submits the transforms resolved by `PreCalc` and performs no computation — unlike the removed `setBoneTransformations`, which did both.

### D4 — Derive descriptive camera state from the raw view matrix

`CameraRawMatrix::setViewMatrix` additionally decomposes the matrix so the inherited state is meaningful: with `V` the view matrix and `W = inverse(V)` the camera world matrix, position is `W[3].xyz`, right/up/front come from the (normalized) basis columns of `W`, and yaw/pitch/roll are recovered from that basis using the same conventions the FPS camera uses.

*Alternative considered:* leave `CameraRawMatrix` alone and expose only the view matrix to sections. Rejected — the spec requires position/direction/up to be observable, and every existing expression variable (`cam_posX`, `cam_yaw`, …) is scalar; exposing only a matrix would leave script authors with nothing usable.

### D4b — Unify the two producers of a model camera's view matrix

A decomposition is only well-defined if its input has one convention, and the two producers disagreed:

- `ReadNodeHeirarchy` passes `glm::inverse(GlobalTransformation)` — an OpenGL-style view matrix, **-Z forward**.
- `processCameras` passed `aiCamera::GetCameraMatrix`, which writes `zaxis = mLookAt` (the look *direction*) into row 2 and `xaxis = mUp ^ mLookAt`. That is **+Z forward**, with the X axis also negated relative to `glm::lookAt`'s `cross(f, up)` — a 180° rotation about Y. Assimp's own comment on that method reads *"todo: test ... should work, but i'm not absolutely sure"*.

So `processCameras` is changed to build the view matrix directly with `glm::lookAt(pos, pos + lookAt, up)`, matching the animated path. Verified numerically: the old matrix yielded a front vector 174.6° away from the model's true look direction, the new one reproduces it exactly.

This is a fix, not merely a refactor, and it is tightly coupled to the change: `resolveModelCamera` runs on every `PreCalc` regardless of `playAnimation`, whereas `ReadNodeHeirarchy` only runs for animated models. Without it, `modelcam_front*`, `modelcam_yaw`, `modelcam_pitch` and `modelcam_roll` would be silently backwards for every static model camera, violating the spec requirement that the exposed direction correspond to the camera's placement. It also removes a latent inconsistency in the *rendering* path, where the same static camera produced a mirrored view.

FOV is not recoverable from a view matrix, so it is taken from the model: Assimp reports the *half horizontal* angle, which is converted to the full vertical angle in degrees that `m_Fov` and `glm::perspective` expect, using the camera's declared aspect ratio when present.

**Behavior note:** demos that select a camera from a model with no animations will now see that camera pointing the correct way rather than backwards. This is the one place where rendering output intentionally differs from the pre-change build.

### D5 — Expose model camera values as new expression variables, not by hijacking the active camera

`MathDriver` gains a `modelcam_*` variable family (`modelcam_posX/Y/Z`, `modelcam_frontX/Y/Z`, `modelcam_upX/Y/Z`, `modelcam_yaw`, `modelcam_pitch`, `modelcam_roll`, `modelcam_fov`), bound by reference to storage owned by the `MathDriver` instance. Because exprtk binds by reference at symbol-table build time, the section only needs to write those floats after `PreCalc` and before the next `executeFormula()`.

*Alternative considered:* call `CameraManager::setActiveCamera(model camera)` after `PreCalc` so the existing `cam_*` variables pick it up. Rejected on two counts: it would change the *rendering* view for every later section in the frame (a global side effect from a local draw), and it would silently redefine `cam_*` mid-frame for sections that expect the demo camera.

The `cam_*` family keeps its current meaning (active demo camera) and is untouched.

### D6 — Section per-frame order, with a second expression evaluation

Each affected section's `exec()` becomes:

1. `setRenderStatesStart()` / `EvalBlendingStart()`
2. `m_pExprPosition->executeFormula()` — resolves `AnimationTime` and any value not depending on the model camera
3. apply model properties (`playAnimation`, `setAnimation`, `setCamera`)
4. `PreCalc(m_fAnimationTime)`
5. publish the resolved model camera into the `MathDriver` `modelcam_*` storage
6. `m_pExprPosition->executeFormula()` — now the placement values can consume the model camera
7. build `m_matBaseModel` / projection / view, `m_pVars->setValues()`
8. `Draw(...)` (or `drawInstanced`)

Two evaluations per frame is a deliberate cost. It is the only way to break the circular dependency `AnimationTime → animation → model camera → placement`, and it is a pattern the codebase already uses — `drawSceneMatrix::updateMatrices` evaluates the formula once up front and again inside the per-instance loop.

*Alternative considered:* evaluate once and accept one frame of latency (use the previous frame's model camera). Rejected — it produces visible lag on fast camera moves and is exactly the class of bug this change exists to remove.

For the matrix sections, `PreCalc` is hoisted **out** of the instance loop (step 4 happens once), while `Draw` stays inside it. For the folder sections, `PreCalc` runs once per distinct model, still outside the per-instance loop.

### D7 — Defensive behavior when `PreCalc` was never called

`m_matNodeAnimated` is identity-initialized and `m_boneTransforms` retains its loader-time sizing, so a `Draw` without a prior `PreCalc` renders the model in its bind pose rather than producing garbage or dereferencing unset state. No exception, no assert — consistent with the engine's existing "log and keep rendering" policy for animation errors.

## Risks / Trade-offs

- **Transform composition regression (highest risk).** The `*=` → `=` change is the load-bearing edit; getting the operand order wrong silently mis-places every animated mesh. → Validate against the existing regression assets first (`cubes.glb` from `drawscene-glb-node-transform-fix`, plus an animated model), comparing frames before and after; the composition is algebraically identical, so any visible difference is a bug.
- **Motion blur history double-advance.** `mesh->m_matPrevModel = mesh->m_matModel` and the `m_matPrev*` rollover must stay in `Draw` only. Moving either into `PreCalc` would advance history once per frame instead of once per draw, breaking the matrix sections. → Spec scenario "Precalculation does not advance history" covers this; keep all history writes in `Draw`.
- **Missed `PreCalc` at a call site.** A section that forgets step 4 renders a static bind pose — a quiet, easy-to-miss failure. → Dropping `currentTime` from `Draw`/`drawInstanced` turns "forgot to precalculate" into a signature that no longer carries the time, making the omission obvious at the call site; all five sections are migrated in this change.
- **Euler recovery ambiguity.** Yaw/pitch/roll extracted from a matrix are non-unique at gimbal-lock poses. → Position, front and up are the reliable outputs and are what expressions should prefer; the angles are best-effort and documented as such.
- **Second expression evaluation cost.** One extra exprtk evaluation per section per frame. → Negligible next to the eliminated per-instance hierarchy walks; the net change is strongly positive for animated matrix sections.
- **Non-uniform scale in the camera basis.** Normalizing the basis columns discards scale, which is correct for a camera but would hide a malformed export. → Acceptable; malformed camera nodes are out of scope.

## Migration Plan

Single-commit, engine-internal. Ordering that keeps the tree buildable and each step verifiable:

1. Add `Mesh::m_matNodeAnimated` (identity default) — no behavior change.
2. Add `Model::PreCalc` and switch `ReadNodeHeirarchy` to assign into `m_matNodeAnimated`; move the model-camera resolution into `PreCalc`; reduce `Draw` and update its signature.
3. Add `ModelInstance::PreCalc`; reduce `drawInstanced`.
4. Add the `CameraRawMatrix` decomposition.
5. Add the `modelcam_*` variables to `MathDriver` and the publish step.
6. Migrate the five sections to the new order.
7. Visual regression pass against known demo data.

Rollback: revert the commit. There is no persisted state, no script/shader format change and no data migration, so rollback is unconditional.
