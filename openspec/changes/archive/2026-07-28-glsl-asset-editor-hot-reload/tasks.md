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

## Phase 5: GLSL Error Notifications

- [x] T016 Convert failed and deactivated asset-impact entries into subject-linked Cacablu Events while preserving section error markers.
- [x] T017 Capture Phoenix's recent-log baseline before connected GLSL Preview/Save operations and record newly produced compiler/linker errors after success or failure.
- [x] T018 Add focused unit coverage for structured impact Events from multiple dependent sections, prior-log suppression, detailed diagnostics, subject inference, and rejected operations.
- [x] T019 Verify through the real GLSL Monaco panel that Preview exposes a Phoenix shader error without changing editor text or bringing Events to the foreground.
- [x] T020 Make Phoenix asset reload tolerate a section already absent after an earlier failed load without emitting `Section NOT removed from runtime`.
- [x] T021 Clear resolved section error IDs and scoped Phoenix asset/log Events for `reloadedSections`, while preserving current failed/deactivated sections.
- [x] T022 Add unit and real-panel regression coverage for multiple sections transitioning from shader failure to successful repair.
