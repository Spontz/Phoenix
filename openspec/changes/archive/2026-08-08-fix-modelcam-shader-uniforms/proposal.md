## Why

`modelcam_*` made the expression plumbing harder to reason about, and shader uniform expressions are still evaluated by separate `MathDriver` instances that do not see section-local camera state. The cleaner fix is to reuse the existing `cam_*` family by temporarily pointing the expression camera at the model camera during drawscene evaluation, so both section expressions and shader uniforms read the same values without adding a second camera namespace.

## What Changes

- Remove the dedicated `modelcam_*` expression variables.
- Use the existing `cam_*` variables for both section expressions and shader uniforms while a drawscene section evaluates a model camera.
- Keep the active render camera unchanged; only the expression-facing camera snapshot is swapped temporarily.
- Preserve the current fallback behavior when a model has no valid camera selected.

## Capabilities

### New Capabilities
- None; this is an implementation fix with no intended spec delta. `skip_specs: true` is set for this change.

### Modified Capabilities
- None.

## Impact

Affected code is centered on `CameraManager`, `MathDriver`, `ShaderVars`, and the drawscene family of sections that publish a model camera and evaluate shader uniforms. No shader files or `.spo` syntax changes are expected.
