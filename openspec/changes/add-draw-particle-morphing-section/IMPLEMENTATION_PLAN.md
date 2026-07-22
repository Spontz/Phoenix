# Vertex-First Triangle Sampling Plan

## Objective

Update `drawParticleMorphing` so each model supplies particle positions in this order:

1. Use the first requested unique vertices in mesh traversal order.
2. When the requested count exceeds the unique-vertex count, keep every unique vertex and create only the remaining positions inside mesh triangles.

The source and destination models continue to be sampled independently, and their positions remain paired by particle index.

## Current State

`drawParticleMorphing.cpp` currently flattens `m_uniqueVertices` and calls `sampleUniform()`. It selects a strided subset when the request fits the vertex count and linearly interpolates consecutive vertices when it does not. That interpolation creates points on edges, which conflicts with the updated requirement.

`Mesh` stores the vertex and index buffers required to reconstruct triangles, but they are private. A small read-only geometry API is therefore required before the section can sample triangle interiors.

## Design

### 1. Expose read-only mesh geometry

Add const accessors in `Mesh` for its vertex and index arrays. They are only used during section loading and must not expose mutable storage or alter rendering behavior.

### 2. Build an ordered vertex list and triangle sampler

In `drawParticleMorphing.cpp`, replace `sampleUniform()` with a helper that:

- Flattens `m_uniqueVertices` in model and mesh traversal order.
- Appends the first `min(requestedCount, uniqueVertexCount)` vertex positions unchanged.
- Builds a list of non-degenerate triangles from every mesh's indexed vertex data when more positions are needed.
- Selects fallback triangles with probability proportional to their area, so large triangles are not underrepresented.
- Samples each selected triangle with deterministic pseudo-random barycentric coordinates whose three weights are strictly positive. The samples are therefore inside the triangle, not on its edges.
- Uses a fixed seed derived from the requested count so repeated loads produce the same positions.

If a model needs fallback positions but has no non-degenerate triangles, loading the section fails with a diagnostic that identifies the source or destination model.

### 3. Integrate without changing the particle contract

Use the new helper independently for source and destination. The existing particle construction remains unchanged: `InitPosition` receives source position `i`, `Randomness` receives destination position `i`, and the shader continues to morph by index.

## Files Affected

| File | Change |
| --- | --- |
| `Engine/src/core/renderer/Mesh.h` | Add const accessors for vertices and indices. |
| `Engine/src/core/renderer/Mesh.cpp` | Implement the read-only accessors. |
| `Engine/src/sections/drawParticleMorphing.cpp` | Replace strided/edge interpolation sampling with vertex-first, triangle-interior sampling and error handling. |
| `openspec/changes/add-draw-particle-morphing-section/design.md` | Record the read-only mesh-geometry dependency. |
| `openspec/changes/add-draw-particle-morphing-section/tasks.md` | Track the implementation and verification work below. |

## Verification

Verify each model independently with fixture meshes or demo models covering these cases:

1. Requested count below the unique-vertex count: output is exactly the first requested vertices.
2. Requested count equal to the unique-vertex count: output is exactly the complete ordered vertex list.
3. Requested count above the unique-vertex count: the vertex prefix remains unchanged and every extra position has strictly positive barycentric coordinates on a valid triangle.
4. Repeated loads with the same model and count produce identical output.
5. A model requiring fallback positions with no valid triangles fails cleanly.
6. Build the Engine target and smoke-render a morph using models with different vertex and triangle counts.

## Out of Scope

- Changing the section script format, shader uniforms, `ParticleMesh`, or morphing shader behavior.
- Adding runtime resampling after the section has loaded.
- Changing the order in which model meshes or unique vertices are traversed.
