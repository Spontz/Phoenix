## 1. API Model And Validation

- [x] 1.1 Add a graphics configuration DTO for rendering context and 25 generic FBO rows.
- [x] 1.2 Add validation for context fields, FBO index coverage, FBO dimensions, formats, attachment counts, and filter values.
- [x] 1.3 Add response helpers for normalized config, field-level validation details, and restart-required warnings.

## 2. Phoenix Runtime Apply

- [x] 2.1 Add a runtime apply path that updates window properties and `Window::fboConfig`.
- [x] 2.2 Apply V-sync, window size, aspect ratio, and generic FBO recreation from the accepted config.
- [x] 2.3 Ensure generic FBO recreation does not modify demo effect FBOs or WebRTC preview capture FBOs.
- [x] 2.4 Preserve the previous runtime config if apply fails.

## 3. Disk Persistence

- [x] 3.1 Add a serializer for `data/config/graphics.spo` using `SpoReader` variable names.
- [x] 3.2 Create `data/config` when needed and replace `graphics.spo` atomically where possible.
- [x] 3.3 Extend graphics config loading if needed so persisted `gl_colorDepth` is round-tripped consistently.

## 4. Editor API Endpoints

- [x] 4.1 Add `GET /api/graphics` for current normalized graphics settings.
- [x] 4.2 Add `PUT /api/graphics` for full config replacement.
- [x] 4.3 Return structured errors without partial application on validation, apply, or write failure.

## 5. Verification

- [ ] 5.1 Add unit coverage for validation and `graphics.spo` serialization.
- [x] 5.2 Add endpoint tests or a manual curl script for valid and invalid payloads.
- [ ] 5.3 Manually verify that Cacablu can update Phoenix and that `data/config/graphics.spo` changes on disk.
