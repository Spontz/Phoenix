## 1. Temporary Investigation

- [x] 1.1 Measure the Assimp import independently from Phoenix model conversion.
- [x] 1.2 Confirm repeated `earth.png` forced reloads in the existing Phoenix log.
- [x] 1.3 Identify per-mesh material construction as the source of approximately 8 GiB of repeated texture decoding.

## 2. Simple Production Fix

- [x] 2.1 Load each Assimp material once per model and reuse it during mesh construction.
- [x] 2.2 Remove every temporary probe, timer, memory sampler, and experiment artifact.
- [x] 2.3 Confirm the final production code adds no import-profile or benchmarking infrastructure.

## 3. Phoenix Conversion Efficiency

- [x] 3.1 Transfer completed vertex and index containers into `Mesh` ownership without redundant full-container copies.
- [x] 3.2 Verify GPU uploads and CPU-side consumers such as unique-vertex extraction receive unchanged geometry data.

## 4. Manual Validation

- [x] 4.1 Confirm `planets.fbx` loads promptly without the previous multi-gigabyte transient memory spike.
- [x] 4.2 Verify its translation and rotation animations, materials, node transforms, and cameras remain correct.
- [x] 4.3 Run available animated, skinned, material, camera, and archived `drawScene` node-transform regression checks.
- [x] 4.4 Inspect the final diff for simplicity and readability, then run strict OpenSpec validation.
