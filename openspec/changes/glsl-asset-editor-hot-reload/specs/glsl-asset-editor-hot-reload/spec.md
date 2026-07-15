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

Phoenix SHALL find every loaded section that depends on the affected asset and report which of those sections were reloaded, deactivated, or failed after every editor-driven asset preview, write, delete, unpublish, or move operation.

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

#### Scenario: Reload every section that uses an edited shader

- **GIVEN** several loaded sections reference the same GLSL asset
- **WHEN** Cacablu previews or saves edited content for that asset
- **THEN** Phoenix SHALL attempt to reload every dependent section before completing the asset operation
- **AND** the response SHALL report an individual outcome for every dependent section
- **AND** Cacablu SHALL mark and notify every section whose reload failed.

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

### Requirement: GLSL failures are reported in Cacablu Events

Cacablu SHALL report structured asset-impact failures and newly produced Phoenix compilation or linking errors after connected GLSL Preview and Save operations.

#### Scenario: Preview produces a shader compilation error

- **GIVEN** Phoenix is connected and the selected GLSL asset is used by a loaded section
- **AND** the user introduces invalid GLSL content
- **WHEN** the user invokes Preview
- **THEN** Cacablu SHALL add the new Phoenix compiler or linker diagnostic to Events as an error
- **AND** Cacablu SHALL mark every dependent section returned in `failedSections` as erroneous
- **AND** existing Events notification behavior SHALL expose the unread error without activating the Events panel in front of the current panel.

#### Scenario: Save produces a shader compilation error

- **GIVEN** Phoenix is connected and a GLSL Save persists content that cannot reload a dependent section
- **WHEN** Phoenix completes or rejects the persisted asset operation
- **THEN** Cacablu SHALL record the new Phoenix diagnostic in Events
- **AND** SHALL retain the locally persisted shader according to the existing Save behavior.

#### Scenario: Structured impact reports a failed or deactivated section

- **WHEN** an asset operation returns entries in `failedSections` or `deactivatedSections`
- **THEN** Cacablu SHALL add an error Event for every failed section and a warning Event for every deactivated section
- **AND** each Event SHALL retain its section ID as the subject.

#### Scenario: Earlier Phoenix errors exist

- **GIVEN** Phoenix's recent-log buffer contains errors from before the GLSL operation
- **WHEN** Cacablu captures logs for Preview or Save
- **THEN** Cacablu SHALL add only log entries produced after that operation began.

### Requirement: Repaired GLSL clears resolved section failures

Phoenix and Cacablu SHALL treat a successful dependent-section reload after a GLSL failure as recovery of that section.

#### Scenario: Repair shader after a failed reload

- **GIVEN** an earlier invalid GLSL preview left a dependent section inactive and marked as erroneous
- **WHEN** the user repairs the GLSL and previews or saves it again
- **THEN** Phoenix SHALL accept that the old runtime section is already absent
- **AND** Phoenix SHALL attempt to load the repaired section without emitting a removal error
- **AND** the successful section SHALL appear in `reloadedSections`.

#### Scenario: Cacablu receives successful repaired sections

- **WHEN** an asset-impact response includes previously failed section IDs in `reloadedSections`
- **THEN** Cacablu SHALL clear those section IDs from its Timeline error state
- **AND** Cacablu SHALL remove the resolved Phoenix asset and log Events associated with those sections
- **AND** sections still present in `failedSections` or `deactivatedSections` SHALL remain erroneous.
