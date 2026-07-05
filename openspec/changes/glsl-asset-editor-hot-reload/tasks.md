## Phase 1: API Contract

- [x] T001 Add request/response DTOs for transient asset preview and asset impact responses.
- [x] T002 Validate normalized asset paths so only `pool` and `resources` assets can be previewed or impacted.
- [x] T003 Add `PUT /api/assets/preview` to the editor API router.

## Phase 2: Runtime Asset Overlay

- [x] T004 Add an in-memory asset overlay keyed by normalized data-relative path.
- [x] T005 Make GLSL/text asset loading consult the overlay before disk.
- [x] T006 Clear overlay entries when the same asset is committed, deleted, moved, or the data folder changes.

## Phase 3: Section Dependency And Impact

- [x] T007 Build section-to-asset and asset-to-section dependency indexes when sections load.
- [x] T008 Refresh dependency index entries after single-section updates and full section replacement.
- [x] T009 Reload dependent sections after preview or persisted write operations.
- [x] T010 Deactivate dependent sections after delete, unpublish, or move-away operations.
- [x] T011 Include reloaded, deactivated, and failed section IDs in all relevant asset responses.

## Phase 4: Verification

- [ ] T012 Add tests or a manual harness for previewing a GLSL asset without disk writes.
- [ ] T013 Verify persisted writes clear preview overrides and reload dependent sections.
- [ ] T014 Verify delete/unpublish deactivates dependent sections and reports their IDs.
- [ ] T015 Verify invalid paths cannot escape `data` or target `config`.
