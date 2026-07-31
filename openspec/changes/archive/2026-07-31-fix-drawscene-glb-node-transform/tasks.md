## 1. Transform propagation in model import

- [x] 1.1 Update `Model::processNode` to carry accumulated node transform through the hierarchy.
- [x] 1.2 Ensure each node->mesh reference creates a runtime entry that preserves its own node-global transform.
- [x] 1.3 Keep support for multiple nodes referencing the same Assimp mesh index with distinct transforms.

## 2. Runtime matrix composition

- [x] 2.1 Update mesh model assignment so final matrix composes section base transform with node-global transform.
- [x] 2.2 Verify matrix multiplication order against expected world placement (including instanced sections).
- [x] 2.3 Guard animation path to avoid double transforms.

## 2b. Instanced rendering path

- [x] 2b.1 Apply per-mesh node-global transform in `ModelInstance` matrix upload for non-animated instanced draws.
- [x] 2b.2 Keep existing animated instanced matrix upload behavior to avoid introducing double transforms.
- [x] 2b.3 Validate `drawSceneMatrixInstanced` and `drawSceneMatrixInstancedFolder` with GLB multi-node models.

## 3. Diagnostics

- [ ] 3.1 Extend debug data to expose node/mesh transform mapping (at least node identity and transform presence).
- [ ] 3.2 Add a short troubleshooting note for GLB node-transform issues.

## 4. Verification

- [x] 4.1 Reproduce bug with `cubes.glb` and confirm objects are no longer centered at origin.
- [x] 4.2 Validate nested hierarchy composition.
- [x] 4.3 Validate repeated mesh index instancing across multiple nodes.
- [x] 4.4 Run smoke check on existing `drawScene` samples.
- [x] 4.5 Run at least one animation-enabled `drawScene` sample and confirm no double transform artifacts.

## 5. Mesh-dependent sections (Phase 3)

- [x] 5.1 Update `Mesh::loadUniqueVerticesPos` so exported unique positions/normals preserve node-global transforms.
- [x] 5.2 Align `drawParticleMorphing` triangle fallback sampling with node-global transformed mesh geometry.
- [x] 5.3 Validate `drawParticlesScene`, `drawEmitterScene`, and `drawEmitterSceneEx` with GLB multi-node assets.
