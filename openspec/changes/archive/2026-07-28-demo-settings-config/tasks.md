## Phase 1: API Model

- [x] T001 Define demo settings request/response DTOs and validation.
- [x] T002 Add log detail option mapping for `0 None`, `1 Essential`, `2 Normal`, and `3 Verbose`.
- [x] T003 Normalize legacy `log_detail 4` to `3 Verbose` when exposing settings.

## Phase 2: Phoenix Endpoint

- [x] T004 Add `GET /api/demo-settings`.
- [x] T005 Add `PUT /api/demo-settings`.
- [x] T006 Apply accepted settings to Phoenix runtime state.
- [x] T007 Persist accepted settings to `data/config/control.spo` atomically.
- [x] T008 Return structured validation, apply, and write errors.

## Phase 3: Verification

- [x] T009 Verify `control.spo` is written with the required lines and order.
- [x] T010 Verify invalid `logDetail` values are rejected.
- [x] T011 Verify `demoEnd` from the payload is applied to runtime and disk.
