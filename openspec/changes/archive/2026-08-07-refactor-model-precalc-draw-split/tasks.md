## 1. Decouple animated state from the per-draw transform

- [x] 1.1 Add `glm::mat4 m_matNodeAnimated` to `Mesh` in `Engine/src/core/renderer/Mesh.h`, identity-initialized in the constructor, documented as the animated node transform produced by the precalculation phase (distinct from `m_matNodeGlobal` and `m_matModel`)
- [x] 1.2 In `Model::ReadNodeHeirarchy`, replace `meshes[i]->m_matModel *= m_matGlobalInverseTransform * GlobalTransformation` with an assignment into `meshes[i]->m_matNodeAnimated`, so the walk no longer depends on or mutates `m_matModel`
- [x] 1.3 Verify by inspection that no other code path reads `m_matModel` between `setMeshesModelTransform` and the draw loop, so the assignment change cannot drop a contribution

## 2. Split `Model` into precalculation and render phases

- [x] 2.1 Add `void Model::PreCalc(float currentTime)` to `Model.h`/`Model.cpp`: when `playAnimation` and the scene has animations, call `boneTransform(currentTime)` to fill `m_boneTransforms`, each mesh's `m_matNodeAnimated`, and each model camera's view matrix
- [x] 2.2 In `PreCalc`, resolve the selected model camera: when `useCamera` is set and `m_currentCamera` is in range, store the resolved view matrix and set a validity flag; otherwise clear the flag
- [x] 2.3 Add read accessors on `Model` for the resolved camera state required by the spec (view matrix, position, front, up, yaw, pitch, roll, fov) plus a query for whether a model camera is currently available
- [x] 2.4 Change `Model::Draw` to `void Draw(SP_Shader shader, uint32_t startTexUnit = 0)`, removing the `currentTime` parameter
- [x] 2.5 In `Draw`, replace `setMeshesModelTransform()` with the per-mesh composition: `m_matModel = m_matBaseModel * (playAnimation ? m_matNodeAnimated : m_matNodeGlobal)`; remove `setMeshesModelTransform` if it has no remaining callers
- [x] 2.6 In `Draw`, apply the model camera view override from the value resolved in `PreCalc` instead of reading `m_camera[m_currentCamera]->getView()` directly
- [x] 2.7 Move the `gBones` uniform upload from `setBoneTransformations` into `Draw`, guarded by a non-empty `m_boneTransforms`; delete `setBoneTransformations` and make `boneTransform` reachable only from `PreCalc`, removing the `// Hack for ModelInstance` public entry point from `Model.h`
- [x] 2.8 Confirm all previous-frame matrix writes (`mesh->m_matPrevModel`, `m_matPrevProjection`, `m_matPrevView`) remain exclusively in `Draw` and are absent from `PreCalc`

## 3. Split `ModelInstance` into precalculation and render phases

- [x] 3.1 Add `void ModelInstance::PreCalc(float currentTime)` that calls `m_pModel->PreCalc(currentTime)` and then performs the existing instance-matrix buffer upload done by `updateMatrices()`
- [x] 3.2 Change `ModelInstance::drawInstanced` to `void drawInstanced(SP_Shader shader, uint32_t startTexUnit = 0)`, removing the `currentTime` parameter and the `setBoneTransformations` call, leaving only material binding, VAO bind and `glDrawElementsInstanced`
- [x] 3.3 Keep `updateMatrices()` public as the buffer-upload primitive and confirm the `playAnimation` branch inside it is preserved unchanged

## 4. Make model camera state usable by expressions

- [x] 4.1 In `CameraRawMatrix::setViewMatrix`, decompose the inverse view matrix to populate the inherited `m_Position`, `m_Front`, `m_Up`, `m_Right` from its normalized basis, so the camera no longer reports constructor defaults
- [x] 4.2 Recover `m_Yaw`, `m_Pitch` and `m_Roll` from that basis using the same angle conventions as `CameraProjectionFPS`, keeping the result stable at gimbal-lock poses
- [x] 4.3 Add a `modelcam_*` variable family to `MathDriver` (`posX/Y/Z`, `frontX/Y/Z`, `upX/Y/Z`, `yaw`, `pitch`, `roll`, `fov`) bound by reference to storage owned by the `MathDriver` instance, defaulting to neutral values so existing expressions are unaffected
- [x] 4.4 Add a `MathDriver` method that copies a model's resolved camera state into that storage, to be called by sections between precalculation and expression evaluation
- [x] 4.5 Confirm the existing `cam_*` family still resolves to the active demo camera and that no section changes the active camera as a side effect of drawing a model

## 5. Migrate the sections to the two-phase order

- [x] 5.1 `drawScene.cpp`: reorder `exec()` to evaluate the expression, apply model properties, `PreCalc(m_fAnimationTime)`, publish the model camera, re-evaluate the expression, set matrices and shader variables, then `Draw`
- [x] 5.2 `drawSceneMatrix.cpp`: apply model properties and call `PreCalc` once, outside the per-instance loop; keep `Draw` inside the loop with the per-instance `m_matBaseModel`; publish the model camera before the expression evaluation that feeds `updateMatrices`
- [x] 5.3 `drawSceneMatrixFolder.cpp`: call `PreCalc` once per distinct model, outside its per-instance loop; keep `Draw` inside the loop
- [x] 5.4 `drawSceneMatrixInstanced.cpp`: call `ModelInstance::PreCalc` in place of the current `updateMatrices()` call and drop `currentTime` from the `drawInstanced` call
- [x] 5.5 `drawSceneMatrixInstancedFolder.cpp`: same migration, applied per model instance in the folder loop
- [x] 5.6 Verify every affected section's `warmExec()` still routes through the migrated `exec()` so warm-up follows the same phase order

## 6. Build and validate

- [x] 6.1 Build the Engine solution and resolve every compilation error caused by the changed `Draw` and `drawInstanced` signatures, confirming there are no remaining callers passing `currentTime`
- [x] 6.2 Render the `cubes.glb` regression case from `drawscene-glb-node-transform-fix` and confirm the cubes stay separated per node transform
- [x] 6.3 Render an animated model through `drawScene` with `playAnimation` enabled and confirm the animation is visually identical to the pre-change build
- [x] 6.4 Render an animated model through `drawSceneMatrix` with many instances and confirm all instances share the same pose while keeping distinct placement
- [x] 6.5 Render an animated model through `drawSceneMatrixInstanced` and confirm the instanced path is unchanged
- [x] 6.6 Verify motion blur still resolves correctly in a section that uses `prev_model` / `prev_MVP`, with no ghosting or frozen history
- [x] 6.7 Author a test expression that drives model placement from a `modelcam_*` value and confirm the geometry reacts within the same frame, with no one-frame lag
- [x] 6.8 Confirm the invalid-camera and invalid-animation paths still log once per distinct bad index rather than once per frame
