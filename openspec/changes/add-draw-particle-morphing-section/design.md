## Context

Phoenix organizes demo content as temporal "sections" that inherit from `Section` and implement `load/init/exec/warmExec/loadDebugStatic`. Each section type is registered in `kSectionInfo` (in `SectionManager.cpp`) as a tuple of string id, `SectionType` enum value, and `instance_*` factory function declared in `sections.h`. The existing particle section `drawParticlesScene` loads one model, samples its `m_uniqueVertices` as emitter positions, generates shell particles, builds a `ParticleMesh`, and renders it with a shader whose uniforms are applied through `ShaderVars`.

`ParticleMesh::Particle` exposes `InitPosition`, `Randomness`, `InitColor`, `Life`, `Type`, and `ID`. The `Randomness` attribute is already passed to the shader as a per-particle attribute, so it can carry the destination position without any change to `ParticleMesh`. `Mesh::m_uniqueVertices` (vector of `UniqueVertex{Position, PositionPolar, Normal}`) is the source of surface points per model, populated by `loadUniqueVerticesPos()`. Models are loaded through `ModelManager::addModel()` and shaders through `ShaderManager::addShader()`, both rooted at `m_demo.m_dataFolder`. Section shaders live per-section under `Launcher/data/resources/shaders/sections/<sectionName>/`.

## Goals / Non-Goals

**Goals:**
- Add a `drawParticleMorphing` section that morphs a particle cloud between two 3D models.
- Load two independent models (source and destination) and sample each surface uniformly with the same requested particle count, even when the models have completely different vertex counts and topology.
- Pair source and destination positions by index so particle `i` morphs from `source[i]` to `destination[i]`, with the pairing arbitrary in vertex identity but uniform in coverage.
- Expose `fProgress`, `fDuration`, and `iNumParticles` plus user-declared uniforms so the transition curve is fully controlled from the shader.
- Store the section shader under `resources/shaders/sections/drawParticleMorphing/` following the existing convention.
- Integrate with the existing `SectionManager` pipeline and `.spo` publishing like any other draw section.

**Non-Goals:**
- Change `ParticleMesh`, `ModelManager`, `ShaderManager`, or any existing section.
- Persist morph state across section reloads beyond what `Section` already does.
- Support more than two models per section (no multi-stage morphing in this change).
- Provide a built-in easing library in C++; easing is expressed in the shader using user-declared uniforms.
- Guarantee a specific sampling algorithm; the contract is uniform coverage and determinism, not a named method.

## Decisions

### Reuse `drawParticlesScene` as the structural template

The new section will follow `drawParticlesScene` for load/init/exec/warmExec/loadDebugStatic/debug/destructor shape, `MathDriver` run expression with `tx/ty/tz/rx/ry/rz/sx/sy/sz`, `ShaderVars` uniform application, render states, and blending. This keeps the new section consistent with the rest of the engine and minimizes new surface area.

Alternative considered: build the section from scratch on top of raw `Section`. That would diverge from the established particle section patterns and increase review/maintenance cost.

### Carry the destination position in `ParticleMesh::Particle::Randomness`

`ParticleMesh` already passes `Randomness` to the shader as a per-particle attribute. Setting `InitPosition = source[i]` and `Randomness = destination[i]` lets the shader mix the two with `fProgress` without any change to `ParticleMesh`. This reuses the existing attribute channel and avoids a new buffer.

Alternative considered: extend `ParticleMesh` with a dedicated destination attribute. That would require changing `ParticleMesh`, the particle shader inputs, and every existing particle shader, which is out of scope for this change.

### Sample source and destination independently and pair by index

Because the two models can have unrelated vertex counts and topology, the section computes the requested number of positions for each model independently, then pairs them by index. The pairing is arbitrary with respect to vertex identity but uniform in coverage on each surface, so the morph reads as an even flow of particles between the two shapes rather than a stretch between corresponding vertices.

Alternative considered: match vertices by spatial proximity or by a correspondence heuristic. That is expensive, fragile for unrelated topology, and unnecessary because uniform coverage already produces a visually correct morph.

### Uniform sampling with deterministic fallback

For each model, the section collects all `m_uniqueVertices` across its meshes, then:
- If the requested count is less than or equal to the total unique vertex count, it selects a uniform subset using a deterministic stride over the flattened unique vertex list (so repeated loads produce the same distribution).
- If the requested count exceeds the total unique vertex count, it generates additional positions by interpolating between consecutive unique vertices (staying on the surface edges) with a deterministic jitter seeded by index, so the extra points remain on or near the surface and the result is deterministic.

Allocation across multiple meshes is proportional to each mesh's contribution to the total unique vertex count, so no mesh is over- or under-sampled.

Alternative considered: use a random point-on-triangle sampler weighted by triangle area. That gives true area-uniform coverage but requires triangle indices and area computation, is non-deterministic without a fixed seed, and is more complex than needed for the visual goal. The unique-vertex-based sampler is simpler, deterministic, and visually uniform enough for a morph effect.

### Expose `fProgress` as normalized run time and let the shader own the curve

The section computes `fProgress = runTime / duration` in `[0,1]` and passes it plus `fDuration` and `iNumParticles` every frame. Any easing (linear, ease-in, ease-out, bounce, stepped, or arbitrary) is expressed in the shader by combining `fProgress` with user-declared uniforms read through `ShaderVars::ReadString` and applied through `ShaderVars::setValues`. This keeps the section code curve-agnostic.

Alternative considered: implement easing curves in C++ selectable by a parameter. That would limit the curve set, require section code changes for new curves, and duplicate work the shader can already do.

### Shader under `resources/shaders/sections/drawParticleMorphing/`

The section shader lives in its own folder under `resources/shaders/sections/`, matching every other section (e.g. `drawParticlesScene/`, `drawEmitterScene/`). The `shaderBlock[0]->filename` references the shader as a relative path resolved under `m_demo.m_dataFolder`, consistent with the other sections.

Alternative considered: place the shader in a shared `particles` folder. That would break the per-section convention and make per-section shader customization harder.

## Risks / Trade-offs

- Uniform coverage via unique-vertex sampling is not area-uniform -> acceptable for a morph effect; if a future demo needs area-uniform coverage, the sampler can be swapped without changing the section contract.
- Index pairing between unrelated models can produce visually uneven flows if one model is sampled much more densely in one region -> mitigated by proportional per-mesh allocation and deterministic stride; the spec only requires uniform coverage, not a specific pairing.
- Generating extra positions when the count exceeds unique vertices can place points slightly off the surface -> interpolation stays on surface edges and jitter is small; the spec requires "on or near the surface".
- Adding a new `SectionType` enum value is a source-level change that must stay in sync across `Section.h`, `sections.h`, `SectionManager.cpp` (`kSectionInfo` and `getSectionType`) -> the task list enforces updating all three in one group.
- The default shader must declare `fProgress`, `fDuration`, `iNumParticles`, and the user easing uniform, and must use `Randomness` as the destination -> covered by a dedicated shader task and verified by a build + smoke render.

## Migration Plan

1. Add `DrawParticleMorphing` to the `SectionType` enum, declare `instance_drawParticleMorphing` in `sections.h`, and register it in `kSectionInfo` plus `getSectionType` in `SectionManager.cpp`.
2. Implement `Engine/src/sections/drawParticleMorphing.cpp` with the two-model load, independent uniform sampling, `ParticleMesh` initialization, exec with transition uniforms, debug, and cleanup.
3. Add the `Launcher/data/resources/shaders/sections/drawParticleMorphing/` folder with the default morphing shader.
4. Add a demo data folder with two test models and a `drawParticleMorphing` section script so the effect can be smoke-tested.
5. Build the Engine target and run a smoke render of the new section to confirm particles morph from source to destination across the section duration.
6. Confirm existing sections still load and render unchanged (no regressions in `SectionManager` or shared managers).
