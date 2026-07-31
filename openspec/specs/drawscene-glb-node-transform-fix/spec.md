## Purpose
Fix `drawScene` so GLB files preserve per-node object transforms exported by Blender (including files exported with "Apply Transformations"). Distinct objects in the same GLB must keep their spatial offsets instead of collapsing at the origin.

## Requirements

### Requirement: Preserve GLB node transforms in drawScene
Phoenix SHALL preserve the scene-node transforms provided by Assimp when loading static GLB meshes for `drawScene`.

#### Scenario: Multiple transformed objects in one GLB
- **GIVEN** a GLB scene with multiple objects under different node transforms
- **WHEN** `drawScene` loads and renders that model
- **THEN** each mesh is rendered at its node-derived position/orientation/scale
- **AND** objects are not all rendered at world origin unless their node transform is actually identity

#### Scenario: Blender GLB with applied object transforms
- **GIVEN** a GLB exported from Blender where objects were transformed and exported with "Apply Transformations"
- **WHEN** Phoenix renders the model through `drawScene`
- **THEN** object separation from Blender is preserved in Phoenix

### Requirement: Correct hierarchical transform composition
Phoenix SHALL compose mesh transforms from the full node hierarchy, not only the section base transform.

#### Scenario: Deep hierarchy
- **GIVEN** a mesh attached to a child node with transformed ancestors
- **WHEN** model transforms are evaluated
- **THEN** the mesh transform includes parent and child node transforms in correct order

#### Scenario: Section transform still applies globally
- **GIVEN** a valid `drawScene` section expression (`tx,ty,tz,rx,ry,rz,sx,sy,sz`)
- **WHEN** the section executes
- **THEN** the section base transform is applied on top of each mesh node transform
- **AND** moving/scaling/rotating the section affects the whole model as a group

### Requirement: Mesh instances keep distinct transforms
Phoenix SHALL preserve distinct transforms when multiple nodes reference the same source mesh.

#### Scenario: Instanced mesh in GLB hierarchy
- **GIVEN** two or more scene nodes referencing the same Assimp mesh index with different node transforms
- **WHEN** Phoenix builds runtime meshes
- **THEN** runtime draws keep one transform per node instance
- **AND** instances render at distinct locations

### Requirement: Existing material and animation behavior is not regressed
Fixing static node transforms SHALL NOT break current rendering behavior for materials, shaders, and animation pathways.

#### Scenario: Materials and shaders
- **WHEN** a model is rendered after the fix
- **THEN** existing material bindings and shader uniforms continue to work as before

#### Scenario: Animation enabled path
- **WHEN** `drawScene` runs with `playAnimation` enabled on an animated model
- **THEN** runtime behavior remains compatible with current animation pipeline
- **AND** no extra transform is applied that double-transforms animated geometry

### Requirement: Regression coverage for the reported GLB bug
Phoenix SHALL include a reproducible validation case for the reported issue.

#### Scenario: cubes.glb validation
- **GIVEN** the reported `cubes.glb` containing several cubes with distinct transforms
- **WHEN** it is rendered using the provided simple model/view/projection shader in `drawScene`
- **THEN** cubes appear separated in space according to GLB node transforms
- **AND** the previous behavior (all cubes centered) does not occur

### Requirement: Diagnostic visibility
Phoenix SHALL expose enough debug information to verify node-transform handling.

#### Scenario: drawScene debug output
- **WHEN** `drawScene::debug()` is requested
- **THEN** debug data includes per-mesh node identity and transform-related context sufficient to confirm node transform ingestion
- **AND** this information can be used to diagnose future GLB transform import issues
