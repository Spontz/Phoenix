## 1. Section type registration

- [ ] 1.1 Add `DrawParticleMorphing` to the `SectionType` enum in `Engine/src/core/Section.h`.
- [ ] 1.2 Declare `Section* instance_drawParticleMorphing();` in `Engine/src/sections/sections.h`.
- [ ] 1.3 Add the `kSectionInfo` entry mapping `drawParticleMorphing` to `SectionType::DrawParticleMorphing` and `instance_drawParticleMorphing` in `Engine/src/core/SectionManager.cpp`.
- [ ] 1.4 Add the `drawParticleMorphing` string to `getSectionType` in `Engine/src/core/SectionManager.cpp` so editor-published `.spo` files resolve to the new type.

## 2. Section implementation

- [ ] 2.1 Create `Engine/src/sections/drawParticleMorphing.cpp` defining `sDrawParticleMorphing` inheriting from `Section` and `instance_drawParticleMorphing()` returning a `new sDrawParticleMorphing()`.
- [ ] 2.2 Implement `load()` to validate the script contract: exactly 1 numeric parameter (particle count > 0), exactly 2 strings (source and destination model paths), exactly 1 shader block, and 1 run expression; log a structured error and return false on any mismatch.
- [ ] 2.3 In `load()`, load the source model via `m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0])` and the destination model via `addModel(m_demo.m_dataFolder + strings[1])`, call `loadUniqueVertices()` on both, and fail with a logged error if either returns `nullptr` or has zero unique vertices across all meshes.
- [ ] 2.4 In `load()`, load the shader via `m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename)` and return false if it fails.
- [ ] 2.5 Implement a deterministic uniform sampler that, given a model and the requested particle count, collects all `m_uniqueVertices` across its meshes, allocates particles per mesh proportional to each mesh's unique vertex count, selects a uniform subset when the count fits, and interpolates/jitters on the surface when the count exceeds the unique vertices, producing exactly the requested number of positions deterministically.
- [ ] 2.6 Run the sampler independently for the source and destination models so each produces exactly the requested number of positions regardless of the other model's vertex count or topology.
- [ ] 2.7 Build the `ParticleMesh::Particle` vector with `InitPosition = source[i]`, `Randomness = destination[i]`, `Type = Shell`, `ID = i`, and `Life = duration`, then call `m_pParticleMesh->init(Particles)` and return false if initialization fails.
- [ ] 2.8 Implement `init()` to allocate the `MathDriver` and `ShaderVars`, read the shader uniforms via `ShaderVars::ReadString`, and validate the run expression.
- [ ] 2.9 Implement `exec()` to evaluate the run expression, set render states and blending, use the shader, set model/view/projection matrices, set `fProgress = runTime / duration`, `fDuration`, and `iNumParticles`, apply user uniforms via `ShaderVars::setValues`, and render the particle mesh; do nothing when disabled or outside `[startTime, endTime]`.
- [ ] 2.10 Implement `warmExec()` consistent with `exec()` for the warm-up pass.
- [ ] 2.11 Implement `loadDebugStatic()` to report source and destination model filenames, requested particle count, run expression validity, and particle mesh memory in megabytes, and `debug()` to return that string.
- [ ] 2.12 Implement the destructor to delete the `MathDriver`, `ShaderVars`, and `ParticleMesh` if allocated.

## 3. Shader asset

- [ ] 3.1 Create the folder `Launcher/data/resources/shaders/sections/drawParticleMorphing/`.
- [ ] 3.2 Add a default morphing shader in that folder that declares `fProgress`, `fDuration`, `iNumParticles`, and a user easing uniform, uses `InitPosition` as the source and `Randomness` as the destination, and mixes them with `fProgress` (and the easing uniform) to produce the particle position.
- [ ] 3.3 Ensure the shader's uniform declarations match what `ShaderVars::ReadString` expects so user uniforms are applied without section code changes.

## 4. Demo data and smoke test

- [ ] 4.1 Add a demo data folder under `Launcher_WD/data_*` with two test models of different vertex counts and a `drawParticleMorphing` section script referencing them and the new shader.
- [ ] 4.2 Build the Engine target and run a smoke render of the new section to confirm particles morph from source to destination across the section duration.
- [ ] 4.3 Confirm existing sections still load and render unchanged after the `SectionManager` and shared manager changes.

## 5. Verification

- [ ] 5.1 Verify the section fails to load with a structured error for each contract violation (wrong param count, wrong string count, missing shader block, missing expression, particle count <= 0, model load failure, model with no unique vertices, shader load failure).
- [ ] 5.2 Verify the sampler produces exactly the requested particle count for both models when counts are equal, less than, and greater than the unique vertex counts, and that repeated loads produce the same distribution.
- [ ] 5.3 Verify `fProgress` advances from 0 to 1 across the section duration and that user-declared easing uniforms are applied each frame.
- [ ] 5.4 Verify the shader is loaded only from `resources/shaders/sections/drawParticleMorphing/` and not from any other location.
- [ ] 5.5 Run the relevant Phoenix build/test target to confirm no regressions.
