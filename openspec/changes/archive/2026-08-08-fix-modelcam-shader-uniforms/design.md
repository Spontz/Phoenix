## Context

See [proposal.md](proposal.md) - Why.

`MathDriver` already binds `cam_*` variables by reference to `CameraManager::m_pActiveCameraExprTk`, and every section also creates additional `MathDriver` instances inside `ShaderVars` for uniform expressions. Those evaluators are independent, but they all read the same camera snapshot, so changing that snapshot for the duration of a drawscene section is enough to make `cam_*` visible everywhere without introducing a second namespace.

## Goals / Non-Goals

**Goals:**

- Make the model camera visible consistently through the existing `cam_*` expressions in both section placement formulas and shader uniform formulas.
- Avoid adding a second camera variable family or a separate per-section camera store.
- Keep the fix local to camera-expression plumbing and drawscene section flow.

**Non-Goals:**

- Changing how model cameras are resolved from models.
- Changing expression syntax or introducing `modelcam_*` again.
- Reworking the shader variable parser or the model render pipeline.

## Decisions

### D1 — Reuse the existing expression camera snapshot

Instead of adding `modelcam_*`, the drawscene section should temporarily copy the resolved model camera into `CameraManager::m_pActiveCameraExprTk`, because `MathDriver` already binds `cam_*` to that snapshot by reference.

This makes every `MathDriver` in the section see the same camera values automatically, including the ones created inside `ShaderVars`.

*Alternative considered:* add a second `modelcam_*` namespace and keep both camera sources alive. Rejected because it duplicates state and makes the expression model harder to explain.

### D2 — Keep synchronization at the section boundary

The section should publish the resolved model camera into the active expression camera once per frame, after `PreCalc()` and before any expression evaluation that depends on it.

That keeps the update order explicit:

1. resolve model camera
2. copy it into the expression camera snapshot
3. evaluate placement expression
4. evaluate shader uniform expressions

*Alternative considered:* make `ShaderVars` query the model directly. Rejected because `ShaderVars` should remain a pure expression/uniform helper and not grow model-specific logic.

### D3 — Preserve current neutral/fallback semantics

When no valid model camera exists, the expression camera should keep the existing camera values rather than inventing a special uniform path.

*Alternative considered:* reset the active expression camera to zero on every frame without a camera. Rejected because it would diverge from the current expression behavior and would be a broader semantic change than needed for this fix.

## Risks / Trade-offs

- [Shared state lifetime] If the context is stored on `Section`, all `MathDriver` instances must respect that ownership and not outlive the section.
- [Behavior coupling] Reusing the active expression camera means every evaluator in a section sees the same camera snapshot, which is desirable here but means drawscene sections must restore the previous snapshot after rendering.
- [Validation surface] The fix touches the common expression plumbing, so the drawscene variants need regression checks even though only one bug was reported.

## Migration Plan

1. Add the shared model-camera context to the section/expression plumbing.
2. Add a save/restore path in `CameraManager` for the expression-facing active camera snapshot.
3. Keep the drawscene family publishing the resolved model camera once per frame.
4. Validate the reported shader-uniform case and the existing drawscene paths that already use `modelcam_*` in placement expressions.

## Open Questions

None.
