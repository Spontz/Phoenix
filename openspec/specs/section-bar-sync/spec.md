## Purpose
Define how Cacablu project bars are represented as Phoenix runtime sections, compared through the editor API, synchronized during project load, updated individually after edits, and persisted as root `.spo` files in Phoenix's active `data` folder.

## Requirements

### Requirement: Engine section manifest
Phoenix SHALL expose a manifest of the current runtime sections so Cacablu can compare them with project database bars.

#### Scenario: Section manifest is requested
- **WHEN** Cacablu requests the engine section manifest
- **THEN** Phoenix returns a structured list of current sections
- **AND** each section entry includes stable identifier, section type, start time, end time, enabled state, layer, blend factors, blend equation, and section script body or canonical script hash

#### Scenario: Section manifest has deterministic ordering
- **WHEN** Phoenix serializes the current runtime sections
- **THEN** the manifest order is deterministic by section identifier or runtime script order
- **AND** Cacablu can compare the manifest without false differences caused only by response ordering

### Requirement: Initial project bar synchronization
Cacablu SHALL synchronize project database bars to Phoenix runtime sections when a project opens.

#### Scenario: Project bars already match Phoenix sections
- **WHEN** Cacablu opens a project and the serialized project bars exactly match Phoenix's section manifest
- **THEN** Cacablu does not ask Phoenix to delete or recreate sections
- **AND** Phoenix keeps the existing runtime sections

#### Scenario: Project bars differ from Phoenix sections
- **WHEN** Cacablu opens a project and Phoenix has any missing, extra, or changed section relative to the serialized project bars
- **THEN** Cacablu sends a full section replacement request to Phoenix
- **AND** Phoenix deletes all current runtime sections
- **AND** Phoenix creates runtime sections for every project bar in the request
- **AND** Phoenix writes one root `.spo` file under the active `data` folder for every received section
- **AND** Phoenix rebuilds the section load/execution queues using the existing section pipeline

#### Scenario: Section sync follows asset sync
- **WHEN** a project-open sync requires both pool asset uploads and section replacement
- **THEN** Cacablu completes or skips the initial pool synchronization before requesting section replacement
- **AND** section scripts that reference published pool assets can resolve those files from Phoenix's active `data/pool`

### Requirement: Bar to section serialization
Cacablu SHALL serialize database bars into Phoenix-compatible section payloads.

#### Scenario: Bar is serialized
- **WHEN** Cacablu prepares a bar for Phoenix
- **THEN** the payload includes the bar id, type, start time, end time, enabled flag, layer, source blend factor, destination blend factor, blend equation, and raw script text
- **AND** empty or whitespace-only bar type is treated as unconfigured and is not sent to Phoenix
- **AND** the canonical text is equivalent to a root `.spo` section with `:::<type>`, `id`, `start`, `end`, `enabled`, `layer`, `blend`, `blendequation`, a blank line, and the script body

#### Scenario: Invalid bar payload is rejected
- **WHEN** Cacablu sends a section replacement containing an invalid id, missing timing, invalid blend metadata, unsupported type, or malformed script
- **THEN** Phoenix rejects the replacement with a structured error
- **AND** Phoenix does not partially replace the current runtime sections

### Requirement: Section replacement API
Phoenix SHALL provide an editor API operation that atomically replaces the runtime section set from a Cacablu bar snapshot.

#### Scenario: Full replacement succeeds
- **WHEN** Phoenix receives a valid full section replacement request
- **THEN** Phoenix applies the replacement through existing section parsing/loading/event behavior where practical
- **AND** Phoenix emits a structured section change event over the editor WebSocket
- **AND** the event identifies the replacement operation and resulting section count

#### Scenario: Full replacement fails
- **WHEN** Phoenix cannot parse, create, or load one or more requested sections
- **THEN** Phoenix returns a structured error with a stable code and human-readable message
- **AND** Phoenix leaves the previous runtime section set intact when possible

#### Scenario: Replacement is requested outside slave editor mode
- **WHEN** Phoenix is not running the editor API in slave mode
- **THEN** no browser section replacement endpoint is available
- **AND** standalone Phoenix behavior remains unchanged

### Requirement: Single section update API
Phoenix SHALL provide an editor API operation that updates one runtime section and its root `.spo` file without requiring a full section replacement.

#### Scenario: Single section update succeeds
- **WHEN** Phoenix receives a valid single-section update for section id `17`
- **THEN** Phoenix replaces the runtime section for id `17`
- **AND** Phoenix rewrites `<active-data-folder>/17.spo`
- **AND** Phoenix leaves unrelated runtime sections and root `.spo` files intact

#### Scenario: Single section update fails to load
- **WHEN** Phoenix receives a syntactically valid single-section update whose section cannot load
- **THEN** Phoenix returns a structured response that identifies section id `17` as failed
- **AND** the received section content remains available on disk for further editor correction

### Requirement: Section file persistence
Phoenix SHALL persist received editor sections as root `.spo` files in the active `data` folder.

#### Scenario: Section file is written
- **WHEN** Phoenix accepts a section replacement containing a section with internal id `17`
- **THEN** Phoenix writes that section to `<active-data-folder>/17.spo`
- **AND** the file is directly under the active `data` folder, not under `config`, `pool`, or `resources`

#### Scenario: Section file content is formatted
- **WHEN** Phoenix writes a received section to disk
- **THEN** the file starts with `:::<type>`
- **AND** it contains `id <id>`, `start <start>`, `end <end>`, `enabled <0-or-1>`, `layer <layer>`, `blend <srcBlending> <dstBlending>`, and `blendequation <blendingEQ>`
- **AND** it contains one blank line before the raw section script body

#### Scenario: Stale section files are removed on full replacement
- **WHEN** Phoenix applies a full section replacement
- **THEN** Phoenix deletes root `.spo` files that correspond to sections removed from the runtime section set
- **AND** the active `data` root contains one editor-published root `.spo` file per received section id

#### Scenario: Section is deleted
- **WHEN** Phoenix deletes a runtime section with internal id `17` through an editor section operation or full replacement
- **THEN** Phoenix deletes `<active-data-folder>/17.spo` when that file belongs to the deleted section
- **AND** the section is not left represented on disk after it has been removed from runtime
