## Why

Phoenix already has particle-based sections (`drawParticlesScene`, `drawParticles`, `drawParticlesImage`) that sample a single 3D model and render a particle cloud from it. There is no section that morphs a particle cloud between two unrelated 3D scenes, which is a common effect for transitions, logo reveals, and shape-to-shape animations. Because the source and destination models can have completely different vertex counts and topology, the morph cannot rely on matching vertex indices; it needs an independent uniform sampling of each surface so the cloud evenly covers both shapes and the transition reads as an even flow of particles rather than a stretch between corresponding vertices.

## What Changes

- Add a new Phoenix section type `drawParticleMorphing` registered in `kSectionInfo` with a new `SectionType::DrawParticleMorphing` enum value and an `instance_drawParticleMorphing` factory function.
- Add the section implementation `Engine/src/sections/drawParticleMorphing.cpp` following the `drawParticlesScene` pattern but loading two models (source and destination) and generating a uniform particle distribution on each.
- Generate exactly the requested number of particle positions per model, computed independently for source and destination, with deterministic uniform sampling that handles both the case where the count fits the unique vertices (uniform subset) and the case where it exceeds them (interpolation/jitter on the surface).
- Build a `ParticleMesh` where each particle's `InitPosition` is the source position and `Randomness` is the destination position, so the shader can morph between them through the existing randomness attribute.
- Expose built-in transition uniforms `fProgress` (normalized run time in `[0,1]`), `fDuration`, and `iNumParticles`, plus any user-declared uniforms through `ShaderVars`, so the transition curve (linear, ease-in, ease-out, bounce, stepped, or arbitrary) is fully controlled from the shader without section code changes.
- Add the section shader folder `Launcher/data/resources/shaders/sections/drawParticleMorphing/` with a default morphing shader that mixes `InitPosition` and `Randomness` using `fProgress` and a user easing uniform.
- Integrate the section with the existing `SectionManager` pipeline (load/init/warm/exec/loadDebugStatic), standard blending/layer/enabled/timing properties, and `.spo` publishing.

## Capabilities

### New Capabilities
- `draw-particle-morphing`: A Phoenix runtime section that morphs a uniformly distributed particle cloud between two independently sampled 3D models, with shader-driven transition easing.

### Modified Capabilities

## Impact

- `Engine/src/core/Section.h`: add `DrawParticleMorphing` to the `SectionType` enum.
- `Engine/src/sections/sections.h`: declare `Section* instance_drawParticleMorphing();`.
- `Engine/src/sections/drawParticleMorphing.cpp`: new section implementation (load/init/exec/warmExec/loadDebugStatic/debug/destructor).
- `Engine/src/core/SectionManager.cpp`: add the `kSectionInfo` entry and the `getSectionType` mapping for `drawParticleMorphing`.
- `Launcher/data/resources/shaders/sections/drawParticleMorphing/`: new shader folder with the default morphing shader and its uniform declarations.
- Demo authoring: new `:::drawParticleMorphing` section type usable in demo scripts and editor-published `.spo` files.
- No changes to existing sections, the `ParticleMesh` class, the `ModelManager`, or the `ShaderManager`; the section reuses them as-is.
