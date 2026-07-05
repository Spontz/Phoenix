## ADDED Requirements

### Requirement: Transient Asset Preview API

Phoenix SHALL expose an editor API endpoint that accepts an asset payload for runtime preview without writing the asset to disk.

#### Scenario: Preview GLSL asset without persistence

- **GIVEN** Phoenix is running and has loaded sections that reference `pool/shaders/example.glsl`
- **WHEN** Cacablu sends `PUT /api/assets/preview` with updated GLSL content for that path
- **THEN** Phoenix SHALL store the content in an in-memory overlay
- **AND** Phoenix SHALL NOT modify any file under `data`
- **AND** Phoenix SHALL reload sections that reference the asset using the overlay content.

#### Scenario: Reject preview outside editable asset roots

- **GIVEN** Cacablu sends a preview request for `config/graphics.spo` or a path escaping `data`
- **WHEN** Phoenix validates the request
- **THEN** Phoenix SHALL return `ok: false` with code `invalid-asset-path`
- **AND** Phoenix SHALL NOT update runtime memory or disk.

### Requirement: Asset Impact Responses

Phoenix SHALL report which sections were reloaded, deactivated, or failed after every editor-driven asset preview, write, delete, unpublish, or move operation.

#### Scenario: Report successful dependent reloads

- **GIVEN** a changed asset is referenced by sections `17` and `23`
- **WHEN** Phoenix processes the asset operation successfully
- **THEN** the response SHALL include `reloadedSections` entries for sections `17` and `23`
- **AND** the response SHALL include empty `failedSections` for those sections.

#### Scenario: Report dependent reload failures

- **GIVEN** a changed asset is referenced by section `17`
- **AND** section `17` cannot be reloaded with the new asset content
- **WHEN** Phoenix processes the asset operation
- **THEN** the response SHALL include section `17` in `failedSections`
- **AND** section `17` SHALL be inactive until it is successfully reloaded later.

### Requirement: Section Dependency Tracking

Phoenix SHALL maintain a dependency index between loaded sections and normalized `pool` or `resources` asset paths.

#### Scenario: Index dependencies after section sync

- **GIVEN** Cacablu sends sections to Phoenix
- **WHEN** Phoenix accepts and loads those sections
- **THEN** Phoenix SHALL record which normalized asset paths each section references
- **AND** later asset operations SHALL use that index to find affected sections.

#### Scenario: Update dependencies after one section changes

- **GIVEN** section `17` is updated through the editor API
- **WHEN** Phoenix successfully loads the new section content
- **THEN** Phoenix SHALL replace the dependency index entries for section `17`.

### Requirement: Persisted Asset Commit Reload

Phoenix SHALL reload dependent sections after persisted asset writes and clear any transient preview for the same path.

#### Scenario: Commit previewed GLSL asset

- **GIVEN** `pool/shaders/example.glsl` has an active in-memory preview override
- **WHEN** Cacablu saves the asset through the persisted asset write API
- **THEN** Phoenix SHALL write the asset to disk
- **AND** Phoenix SHALL remove the in-memory preview override for that path
- **AND** Phoenix SHALL reload sections that reference the asset from the persisted content.

### Requirement: Dependent Section Deactivation

Phoenix SHALL deactivate sections that depend on an asset that is deleted, unpublished, or moved away.

#### Scenario: Unpublish asset used by sections

- **GIVEN** section `17` references `pool/shaders/example.glsl`
- **WHEN** Cacablu unpublishes or deletes that asset
- **THEN** Phoenix SHALL deactivate section `17`
- **AND** Phoenix SHALL include section `17` in `deactivatedSections`
- **AND** section `17` SHALL NOT execute until a later operation restores the asset or updates the section.
