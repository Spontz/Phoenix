## 1. Read-only mesh geometry access

- [x] 1.1 Add const accessors in `Engine/src/core/renderer/Mesh.h` for the vertex and index arrays needed to enumerate mesh triangles.
- [x] 1.2 Implement those accessors in `Engine/src/core/renderer/Mesh.cpp` without changing rendering ownership or mutability.

## 2. Vertex-first sampler

- [x] 2.1 Replace `sampleUniform()` in `Engine/src/sections/drawParticleMorphing.cpp` with a helper that returns the first requested positions from the flattened `m_uniqueVertices` list in model and mesh traversal order.
- [x] 2.2 When the requested count exceeds the unique-vertex count, preserve the full vertex list as the output prefix and enumerate non-degenerate indexed triangles from every mesh for the remaining positions.
- [x] 2.3 Select fallback triangles with area-weighted deterministic pseudo-random sampling and generate each point with strictly positive barycentric weights so it is inside a triangle rather than on an edge.
- [x] 2.4 Return a structured load error identifying the source or destination model when fallback sampling is required but no valid triangle is available.
- [x] 2.5 Invoke the sampler independently for the source and destination models, preserving the existing `InitPosition = source[i]` and `Randomness = destination[i]` pairing.

## 3. Documentation and verification

- [x] 3.1 Update the design and implementation comments to describe the read-only mesh-geometry access and vertex-first triangle-interior algorithm.
- [ ] 3.2 Verify for each model that a request below, equal to, and above its unique-vertex count produces exactly the requested count; in the first two cases the output must match the ordered vertex prefix exactly.
- [ ] 3.3 Verify every fallback point lies strictly inside one of the model's valid triangles, repeated loads are deterministic, and a model with no valid triangles fails cleanly only when fallback is needed.
- [ ] 3.4 Build the Engine target and smoke-render a source/destination pair with different vertex and triangle counts; confirm no regression in existing particle sections.
