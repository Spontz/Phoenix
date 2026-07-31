# animated-model-load-performance Specification

## Purpose
TBD - created by archiving change fix-animated-fbx-load-performance. Update Purpose after archive.
## Requirements
### Requirement: Animated model loading has bounded resource usage
Phoenix SHALL load the `planets.fbx` reference asset through the animated `drawScene` path without pathological elapsed time or transient memory growth.

#### Scenario: Reference FBX load
- **WHEN** `planets.fbx` is loaded through `drawScene` in a Release build
- **THEN** loading completes without the observed minute-long delay
- **AND** process memory does not exhibit the observed multi-gigabyte transient spike

### Requirement: Shared materials are loaded once
Phoenix SHALL load each imported material once per model and reuse its result for every mesh that references it.

#### Scenario: Many meshes reference one textured material
- **WHEN** a model contains multiple meshes that reference the same Assimp material
- **THEN** Phoenix loads that material and its textures once during model conversion
- **AND** every referencing mesh receives equivalent material properties and texture references

#### Scenario: Forced asset reload
- **WHEN** forced texture reload is enabled while a model is loaded
- **THEN** one material is not repeatedly reloaded merely because multiple meshes reference it

### Requirement: Investigation code is temporary
Performance investigation SHALL NOT leave benchmark, memory-sampling, or profiling infrastructure in the production code.

#### Scenario: Change completion
- **WHEN** the implementation is ready for final validation
- **THEN** temporary timing, memory measurement, and A/B experiment code has been removed
- **AND** the remaining change contains only the production fix and essential validation coverage

### Requirement: Animated model output remains correct
Optimizing model loading SHALL preserve the geometry, materials, node transforms, cameras, and animation data required by `drawScene`.

#### Scenario: planets transform animation
- **WHEN** the optimized loader imports and plays `planets.fbx`
- **THEN** all 253 animated model nodes retain their translation animation
- **AND** all 252 nodes with rotation animation retain their rotation animation
- **AND** playback covers the authored key range without visible discontinuities introduced by the loader

#### Scenario: Existing animated and skinned assets
- **WHEN** existing animated or skinned model regression cases are loaded after the optimization
- **THEN** their animation channels and bone influences remain valid
- **AND** rendering output is equivalent to the pre-change path

### Requirement: Phoenix mesh conversion avoids redundant payload copies
Phoenix SHALL transfer imported vertex and index payloads into runtime mesh ownership without avoidable full-container copies.

#### Scenario: Runtime mesh construction
- **WHEN** Phoenix converts an Assimp mesh into a runtime `Mesh`
- **THEN** each completed vertex and index container is transferred into its final CPU-side owner
- **AND** GPU upload and existing CPU-side mesh consumers continue to receive identical data

