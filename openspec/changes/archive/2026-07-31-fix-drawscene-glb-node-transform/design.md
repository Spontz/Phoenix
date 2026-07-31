## Context

Current import path:

1. `Model::Load()` reads scene with Assimp.
2. `Model::processNode()` traverses nodes and pushes meshes.
3. `Model::setMeshesModelTransform()` sets all mesh model matrices to the same `m_matBaseModel`.

Because node transforms are not persisted/applied per mesh instance, distinct object placements inside GLB are lost and all subobjects render at the same origin-relative transform.

## Design

### 1) Persist node-global transform per runtime mesh entry

During recursive node traversal, carry an accumulated node transform:

- `global = parentGlobal * nodeLocal` (Assimp node transform converted to glm).

For each mesh referenced by the node, create a runtime mesh entry that stores:

- mesh geometry/material payload (as today), and
- `nodeGlobalTransform` (new).

Important: do not rely on mesh name alone. Multiple nodes may reference the same Assimp mesh index with different transforms; each reference needs its own runtime draw entry.

### 2) Compose final model matrix per draw

At render time, for each runtime mesh entry:

- `meshModel = m_matBaseModel * nodeGlobalTransform`.

This keeps section-level movement/rotation/scale intact while preserving GLB-authored object offsets.

### 3) Keep animation path safe

Avoid double-transforming animated/skinned geometry:

- Static path should use `nodeGlobalTransform` composition.
- If existing animated path already applies node transforms in bone evaluation, ensure no extra static transform is injected there.

Implementation can gate behavior by model animation usage if needed.

### 4) Debug/diagnostics

Extend debug output to include (or summarize):

- node name,
- mesh name/index,
- whether node transform is identity,
- optional translation extracted from node matrix.

This allows quick verification that imported transforms are present.

## Alternatives Considered

1. Bake node transforms into vertex positions during import.
- Rejected: mutates source geometry, complicates skinning/normal handling, and loses hierarchy semantics.

2. Apply only node local transform (without parents).
- Rejected: fails on nested hierarchies.

3. Keep one draw entry per Assimp mesh index.
- Rejected: breaks cases where one mesh is instanced by multiple nodes with different transforms.

## Risks

- Matrix order mistakes (`base * node` vs `node * base`) can mirror/offset unexpectedly.
- Animated models may receive duplicate transforms if static/animated paths are not separated.
- Existing content that accidentally depended on old broken behavior may shift visually (expected, but should be noted).

## Validation Plan

1. Repro case: provided `cubes.glb` + simple MVP shader in `drawScene`.
- Expected: cubes appear separated, not collapsed.

2. Hierarchy case: nested parent-child transforms.
- Expected: child placement includes parent transform.

3. Shared mesh index / multi-node instance case.
- Expected: each instance keeps distinct transform.

4. Regression checks:
- regular `drawScene` assets still render,
- animation-enabled model does not double-transform.
