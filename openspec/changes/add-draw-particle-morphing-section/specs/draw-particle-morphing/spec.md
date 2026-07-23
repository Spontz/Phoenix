## Purpose
Define a new Phoenix runtime section type `drawParticleMorphing` that morphs a particle cloud between two 3D scenes. During load, the section analyzes two loaded 3D models (source and destination), places particles on their ordered unique vertices first, and generates any remaining positions inside the models' triangles, producing matching source and destination position buffers. During execution, the section renders the particles transitioning from the source positions to the destination positions, driven by the section's normalized run time. The transition curve is exposed to the shader through uniforms so that the easing can be customized (linear, ease-in, ease-out, or any arbitrary curve) without changing the section code.

## Requirements

### Requirement: Section type registration
Phoenix SHALL register a new section type `drawParticleMorphing` in the section registry so it can be used in demo scripts and editor-published `.spo` files.

#### Scenario: Section type is declared
- **WHEN** the section registry is initialized
- **THEN** `kSectionInfo` contains an entry mapping the string id `drawParticleMorphing` to a new `SectionType::DrawParticleMorphing` enum value and an `instance_drawParticleMorphing` factory function
- **AND** `sections.h` declares `Section* instance_drawParticleMorphing();`
- **AND** `Section.h` adds `DrawParticleMorphing` to the `SectionType` enum

#### Scenario: Section is created from script
- **WHEN** the demo script or an editor-published `.spo` file declares a section with type `:::drawParticleMorphing`
- **THEN** Phoenix instantiates the section through `instance_drawParticleMorphing()`
- **AND** the section `type` is set to `SectionType::DrawParticleMorphing`

### Requirement: Section script contract
The `drawParticleMorphing` section SHALL accept a fixed script contract of parameters, strings, shader block, and expression.

#### Scenario: Script contract is validated on load
- **WHEN** the section loads
- **THEN** the section requires exactly 1 numeric parameter: the number of particles to generate
- **AND** the section requires exactly 2 string parameters: the source scene model path and the destination scene model path, in that order
- **AND** the section requires exactly 1 shader block containing the shader filename and its uniform declarations
- **AND** the section requires 1 run expression for positioning the particle cloud
- **AND** if any of these counts is wrong the section fails to load and logs a structured error identifying the expected contract

#### Scenario: Particle count is validated
- **WHEN** the numeric parameter for particle count is less than or equal to 0
- **THEN** the section fails to load and logs an error indicating the particle count must be greater than 0

### Requirement: Source and destination model loading
The section SHALL load both the source and destination 3D scenes through the existing `ModelManager`.

#### Scenario: Models are loaded
- **WHEN** the section loads
- **THEN** the source model is loaded via `m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0])`
- **AND** the destination model is loaded via `m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1])`
- **AND** both models call `loadUniqueVertices()` so that unique vertex positions are available for sampling

#### Scenario: A model fails to load
- **WHEN** either the source or destination model returns `nullptr` from `addModel`
- **THEN** the section fails to load and logs which model path could not be loaded

#### Scenario: A model has no unique vertices
- **WHEN** either the source or destination model has zero unique vertices across all its meshes
- **THEN** the section fails to load and logs that the offending model has no geometry to sample

### Requirement: Vertex-first particle distribution
The section SHALL generate exactly the requested number of particle positions for each model, computed independently for the source and destination models because they may have completely different vertex counts and topology. It SHALL place particles on the model's unique vertices first, preserving their traversal order. Only when the requested count exceeds the available unique vertex count SHALL it generate the remaining positions as pseudo-random points in the interiors of the model's triangles.

#### Scenario: Source and destination vertex counts differ
- **WHEN** the source and destination models have different numbers of unique vertices, different mesh counts, or different topology
- **THEN** the section still produces exactly the requested number of positions for the source model and exactly the requested number of positions for the destination model
- **AND** the distribution on each model is computed independently from the other model, so the vertex count or geometry of one model never constrains the distribution of the other
- **AND** the resulting morph uses the vertex-first distribution of each model without allowing the vertex count or topology of one model to affect the other

#### Scenario: Particle count fits the available unique vertices
- **WHEN** the requested particle count is less than or equal to the total number of unique vertices of a model
- **THEN** particle position `i` is the position of unique vertex `i` in the model traversal order, for every `i` from zero to the requested particle count minus one
- **AND** no triangle-surface positions are generated

#### Scenario: Particle count exceeds the available unique vertices
- **WHEN** the requested particle count is greater than the total number of unique vertices of a model
- **THEN** the first positions are the model's complete ordered unique-vertex list
- **AND** the section generates exactly the remaining positions as pseudo-random samples over the interiors of the model's triangles so that the total number of positions equals the requested particle count
- **AND** each additional position is sampled with barycentric coordinates that can cover the complete triangle area and is not generated by interpolation along a triangle edge
- **AND** the pseudo-random generation is deterministic for a given model and particle count

#### Scenario: Source and destination buffers are aligned by index
- **WHEN** the source and destination position buffers are generated
- **THEN** both buffers contain exactly the requested number of positions
- **AND** the index `i` in the source buffer corresponds to the index `i` in the destination buffer, so that particle `i` morphs from `source[i]` to `destination[i]`
- **AND** the pairing by index is arbitrary with respect to vertex identity because the models are unrelated

#### Scenario: Distribution accounts for all meshes of a model
- **WHEN** a model contains multiple meshes
- **THEN** the ordered unique-vertex list contains the vertices of every mesh in model traversal order
- **AND** any additional triangle-surface samples can be generated from triangles in every mesh
- **AND** this process is applied independently to the source and to the destination model

### Requirement: Particle mesh initialization
The section SHALL build a `ParticleMesh` from the generated source and destination positions.

#### Scenario: Particles are created
- **WHEN** the source and destination position buffers are ready
- **THEN** the section creates one `ParticleMesh::Particle` per requested particle
- **AND** each particle's `InitPosition` is set to the source position
- **AND** each particle's `Randomness` is set to the destination position, so the destination is available to the shader through the existing randomness attribute
- **AND** each particle's `Type` is set to `ParticleMesh::ParticleType::Shell`
- **AND** each particle's `ID` is set to its index
- **AND** each particle's `Life` is set to the section duration

#### Scenario: Particle mesh is initialized
- **WHEN** the particle vector is populated
- **THEN** the section calls `m_pParticleMesh->init(Particles)` and returns false if initialization fails

### Requirement: Shader uniforms for transition control
The section SHALL expose uniforms to the shader so that the transition between source and destination positions can be customized.

#### Scenario: Built-in transition uniforms are set
- **WHEN** the section executes
- **THEN** the shader receives a `float` uniform named `fProgress` containing the normalized section run time in the range `[0, 1]`, computed as `runTime / duration`
- **AND** the shader receives a `float` uniform named `fDuration` containing the section duration in seconds
- **AND** the shader receives an `int` uniform named `iNumParticles` containing the total number of particles

#### Scenario: Transition curve is customizable through shader uniforms
- **WHEN** the shader block declares additional uniforms (for example an easing exponent, a curve type selector, or any user-defined parameter)
- **THEN** those uniforms are read through `ShaderVars::ReadString` and applied through `ShaderVars::setValues` on each execution
- **AND** the shader is free to combine `fProgress` with those uniforms to produce a non-linear transition (ease-in, ease-out, bounce, stepped, or any arbitrary curve) without requiring changes to the section code

#### Scenario: Shader is loaded
- **WHEN** the section loads
- **THEN** the shader is loaded via `m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename)`
- **AND** if the shader fails to load the section returns false

### Requirement: Shader asset location
The section's shader SHALL be stored under the `resources/shaders/sections/drawParticleMorphing` folder, following the same convention as the other section shaders.

#### Scenario: Shader file is located in the section shader folder
- **WHEN** the shader for a `drawParticleMorphing` section is authored or published
- **THEN** the shader file lives under `<active-data-folder>/resources/shaders/sections/drawParticleMorphing/`
- **AND** the `shaderBlock[0]->filename` in the section script references the shader as a relative path that resolves under that folder, consistent with how the other sections reference their shaders (for example `drawParticlesScene` under `resources/shaders/sections/drawParticlesScene/`)

#### Scenario: Section loads the shader from the conventional location
- **WHEN** the section loads the shader through `m_demo.m_shaderManager.addShader`
- **THEN** the resolved path is `<active-data-folder>/resources/shaders/sections/drawParticleMorphing/<shader-file>`
- **AND** the section does not load shaders from any other location

### Requirement: Execution behavior
The section SHALL render the morphing particle cloud on each execution frame.

#### Scenario: Section executes within its time range
- **WHEN** the section is enabled and the demo time is within `[startTime, endTime]`
- **THEN** the section evaluates its run expression, sets the render states and blending, uses the shader, sets the model/view/projection matrices, sets the transition uniforms, and renders the particle mesh
- **AND** the `fProgress` uniform is recomputed on every frame from the current `runTime` and `duration`

#### Scenario: Section executes outside its time range
- **WHEN** the section is disabled or the demo time is outside `[startTime, endTime]`
- **THEN** the section does not render

#### Scenario: Positioning expression is evaluated
- **WHEN** the section executes
- **THEN** the run expression is evaluated through `MathDriver` and the resulting translation, rotation, and scale are applied to the model matrix used to transform the whole particle cloud
- **AND** the expression exposes the same `tx`, `ty`, `tz`, `rx`, `ry`, `rz`, `sx`, `sy`, `sz` variables as the other draw sections

### Requirement: Debug information
The section SHALL provide debug information consistent with the other particle sections.

#### Scenario: Debug static is loaded
- **WHEN** `loadDebugStatic()` is called
- **THEN** the debug string reports the source and destination model filenames, the requested particle count, whether the run expression is valid, and the memory used by the particle mesh in megabytes

#### Scenario: Debug is returned
- **WHEN** `debug()` is called
- **THEN** the section returns the previously loaded debug static string

### Requirement: Resource cleanup
The section SHALL release its allocated resources on destruction.

#### Scenario: Section is destroyed
- **WHEN** the section destructor runs
- **THEN** the section deletes the `MathDriver`, the `ShaderVars`, and the `ParticleMesh` if they were allocated
- **AND** no OpenGL resources owned by the particle mesh are leaked

### Requirement: Integration with existing section pipeline
The section SHALL integrate with the existing section pipeline like any other draw section.

#### Scenario: Section is queued and executed by the pipeline
- **WHEN** the section is added through the editor or demo script
- **THEN** the section is loaded, initialized, warmed, and executed through the same `SectionManager` queues as the other sections
- **AND** the section supports the standard blending, layer, enabled, and timing properties inherited from `Section`

#### Scenario: Section is published as a `.spo` file
- **WHEN** the editor publishes a `drawParticleMorphing` section
- **THEN** the `.spo` file starts with `:::drawParticleMorphing` and contains the standard `id`, `start`, `end`, `enabled`, `layer`, `blend`, and `blendequation` headers followed by the script body
- **AND** the script body contains the particle count parameter, the two model paths, the shader block, and the run expression
