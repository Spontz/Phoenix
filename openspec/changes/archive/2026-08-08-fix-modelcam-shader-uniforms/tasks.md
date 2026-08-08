## 1. Shared model camera plumbing

- [x] 1.1 Add a save/restore path around the expression-facing active camera snapshot for drawscene sections.
- [x] 1.2 Reuse the existing `cam_*` bindings in `MathDriver` so they read from the temporary expression camera snapshot.
- [x] 1.3 Ensure the model camera publish step copies the resolved model camera into the expression snapshot once after model precalculation.

## 2. Section and shader-uniform integration

- [x] 2.1 Update `drawScene` to publish the model camera into the expression snapshot before evaluating placement expressions and shader uniforms, then restore the previous snapshot.
- [x] 2.2 Update the matrix-based drawscene variants to do the same temporary camera swap for both placement expressions and shader uniforms.
- [x] 2.3 Confirm `ShaderVars`-owned expression evaluators automatically observe the swapped `cam_*` values without any extra per-uniform sync calls.

## 3. Validation

- [x] 3.1 Reproduce the reported shader-uniform case and confirm `cam_posX/Y/Z` reflect the model camera while the drawscene section is active.
- [x] 3.2 Verify a section that does not use model cameras still behaves exactly as before.
- [x] 3.3 Run the existing drawscene regressions to confirm the change does not alter placement or render order.
