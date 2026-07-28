## Context

Phoenix organizes demo content as temporal "sections" that inherit from `Section` and implement `load/init/exec/warmExec/loadDebugStatic`. Each section type is registered in `kSectionInfo` (in `SectionManager.cpp`) as a tuple of string id, `SectionType` enum value, and `instance_*` factory function declared in `sections.h`. The existing particle section `drawParticlesScene` loads one model, samples its `m_uniqueVertices` as emitter positions, generates shell particles, builds a `ParticleMesh`, and renders it with a shader whose uniforms are applied through `ShaderVars`.

`ParticleMesh::Particle` exposes `InitPosition`, `Randomness`, `InitColor`, `Life`, `Type`, and `ID`. The `Randomness` attribute is already passed to the shader as a per-particle attribute, so it can carry the destination position without any change to `ParticleMesh`. `Mesh::m_uniqueVertices` (vector of `UniqueVertex{Position, PositionPolar, Normal}`) provides the initial ordered positions per model, populated by `loadUniqueVerticesPos()`. When more positions are required, the mesh triangle data provides the surface samples. Models are loaded through `ModelManager::addModel()` and shaders through `ShaderManager::addShader()`, both rooted at `m_demo.m_dataFolder`. Section shaders live per-section under `Launcher/data/resources/shaders/sections/<sectionName>/`.

## Goals / Non-Goals

**Goals:**
- Add a `drawParticleMorphing` section that morphs a particle cloud between two 3D models.
- Load two independent models (source and destination), place the first particles on their ordered unique vertices, and sample triangle interiors for any remaining particles, even when the models have completely different vertex counts and topology.
- Pair source and destination positions by index so particle `i` morphs from `source[i]` to `destination[i]`, with the pairing arbitrary in vertex identity.
- Expose `fProgress`, `fDuration`, and `iNumParticles` plus user-declared uniforms so the transition curve is fully controlled from the shader.
- Store the section shader under `resources/shaders/sections/drawParticleMorphing/` following the existing convention.
- Integrate with the existing `SectionManager` pipeline and `.spo` publishing like any other draw section.

**Non-Goals:**
- Change `ParticleMesh`, `ModelManager`, `ShaderManager`, or any existing section. This excludes the read-only `Mesh` geometry access required to sample triangle interiors.
- Persist morph state across section reloads beyond what `Section` already does.
- Support more than two models per section (no multi-stage morphing in this change).
- Provide a built-in easing library in C++; easing is expressed in the shader using user-declared uniforms.
- Support distribution rules other than vertex-first placement followed by deterministic triangle-interior sampling.

## Decisions

### Reuse `drawParticlesScene` as the structural template

The new section will follow `drawParticlesScene` for load/init/exec/warmExec/loadDebugStatic/debug/destructor shape, `MathDriver` run expression with `tx/ty/tz/rx/ry/rz/sx/sy/sz`, `ShaderVars` uniform application, render states, and blending. This keeps the new section consistent with the rest of the engine and minimizes new surface area.

Alternative considered: build the section from scratch on top of raw `Section`. That would diverge from the established particle section patterns and increase review/maintenance cost.

### Carry the destination position in `ParticleMesh::Particle::Randomness`

`ParticleMesh` already passes `Randomness` to the shader as a per-particle attribute. Setting `InitPosition = source[i]` and `Randomness = destination[i]` lets the shader mix the two with `fProgress` without any change to `ParticleMesh`. This reuses the existing attribute channel and avoids a new buffer.

Alternative considered: extend `ParticleMesh` with a dedicated destination attribute. That would require changing `ParticleMesh`, the particle shader inputs, and every existing particle shader, which is out of scope for this change.

### Sample source and destination independently and pair by index

Because the two models can have unrelated vertex counts and topology, the section computes the requested number of positions for each model independently, then pairs them by index. The pairing is arbitrary with respect to vertex identity; its only contract is that particle `i` morphs from source position `i` to destination position `i`.

Alternative considered: match vertices by spatial proximity or by a correspondence heuristic. That is expensive and fragile for unrelated topology, while the section only needs stable indexed source and destination buffers.

### Vertex-first sampling with triangle-interior fallback

For each model, the section collects all `m_uniqueVertices` across its meshes in traversal order, then:
- If the requested count is less than or equal to the total unique vertex count, it uses the first requested positions in that ordered list.
- If the requested count exceeds the total unique vertex count, it uses every unique vertex first and generates the remaining positions as deterministic pseudo-random samples inside mesh triangles using barycentric coordinates. This samples triangle interiors rather than triangle edges.

All meshes contribute their vertices to the initial ordered list and their triangles to the fallback surface sampler.

Alternative considered: interpolate between consecutive vertices. That keeps the implementation simple but places every generated position on an edge, leaving triangle interiors empty. Triangle-interior sampling better represents the model surface and still remains deterministic with a fixed seed.

### Expose read-only mesh geometry for triangle sampling

`Mesh` keeps its source vertices and index buffer private, so the section cannot reconstruct triangles from outside the renderer. It will expose const accessors for those collections. The section reads them only while loading its particle positions; it does not change mesh ownership, render data, or `ParticleMesh`.

### Expose `fProgress` as normalized run time and let the shader own the curve

The section computes `fProgress = runTime / duration` in `[0,1]` and passes it plus `fDuration` and `iNumParticles` every frame. Any easing (linear, ease-in, ease-out, bounce, stepped, or arbitrary) is expressed in the shader by combining `fProgress` with user-declared uniforms read through `ShaderVars::ReadString` and applied through `ShaderVars::setValues`. This keeps the section code curve-agnostic.

Alternative considered: implement easing curves in C++ selectable by a parameter. That would limit the curve set, require section code changes for new curves, and duplicate work the shader can already do.

### Shader under `resources/shaders/sections/drawParticleMorphing/`

The section shader lives in its own folder under `resources/shaders/sections/`, matching every other section (e.g. `drawParticlesScene/`, `drawEmitterScene/`). The `shaderBlock[0]->filename` references the shader as a relative path resolved under `m_demo.m_dataFolder`, consistent with the other sections.

Alternative considered: place the shader in a shared `particles` folder. That would break the per-section convention and make per-section shader customization harder.

## Risks / Trade-offs

- The vertex-first distribution follows mesh traversal order, so its coverage can be less visually even than area-uniform sampling -> this is intentional because preserving the first requested vertices is part of the section contract.
- Index pairing between unrelated models can produce visually uneven flows if their traversal orders differ -> the pairing remains intentionally arbitrary because the models have no vertex correspondence.
- Triangle sampling requires valid triangle data for the meshes that supply the extra positions -> models used with more particles than unique vertices must contain renderable triangle geometry.
- Adding a new `SectionType` enum value is a source-level change that must stay in sync across `Section.h`, `sections.h`, `SectionManager.cpp` (`kSectionInfo` and `getSectionType`) -> the task list enforces updating all three in one group.
- The default shader must declare `fProgress`, `fDuration`, `iNumParticles`, and the user easing uniform, and must use `Randomness` as the destination -> covered by a dedicated shader task and verified by a build + smoke render.

## Migration Plan

1. Add `DrawParticleMorphing` to the `SectionType` enum, declare `instance_drawParticleMorphing` in `sections.h`, and register it in `kSectionInfo` plus `getSectionType` in `SectionManager.cpp`.
2. Implement `Engine/src/sections/drawParticleMorphing.cpp` with the two-model load, independent vertex-first and triangle-interior sampling, `ParticleMesh` initialization, exec with transition uniforms, debug, and cleanup.
3. Add the `Launcher/data/resources/shaders/sections/drawParticleMorphing/` folder with the default morphing shader.
4. Add a demo data folder with two test models and a `drawParticleMorphing` section script so the effect can be smoke-tested.
5. Build the Engine target and run a smoke render of the new section to confirm particles morph from source to destination across the section duration.
6. Confirm existing sections still load and render unchanged (no regressions in `SectionManager` or shared managers).
