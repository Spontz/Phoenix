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
Cacablu SHALL synchronize project database bars to Phoenix runtime sections when a project opens and SHALL force full replacement after Phoenix reconnects.

#### Scenario: Project bars already match Phoenix sections during ordinary project open
- **WHEN** Cacablu opens a project outside reconnect recovery and the serialized project bars exactly match Phoenix's section manifest
- **AND** every matching Phoenix manifest entry reports that its runtime section loaded successfully
- **THEN** Cacablu does not ask Phoenix to delete or recreate sections
- **AND** Phoenix keeps the existing runtime sections

#### Scenario: Matching Phoenix section has a runtime load error
- **WHEN** a serialized project bar matches Phoenix's persisted section metadata and content
- **AND** Phoenix's manifest reports that the runtime section did not load successfully
- **THEN** Cacablu sends a full section replacement after project assets and settings are synchronized
- **AND** Cacablu uses the replacement response to associate any repeated load failure with the matching Timeline bar

#### Scenario: Phoenix does not report section runtime state
- **WHEN** a serialized project bar matches a section manifest entry from an older Phoenix version without runtime load state
- **THEN** Cacablu treats that section state as unknown
- **AND** Cacablu sends a full section replacement instead of assuming the section is healthy

#### Scenario: Project bars differ from Phoenix sections
- **WHEN** Cacablu opens a project and Phoenix has any missing, extra, or changed section relative to the serialized project bars
- **THEN** Cacablu sends a full section replacement request to Phoenix
- **AND** Phoenix deletes all current runtime sections
- **AND** Phoenix creates runtime sections for every project bar in the request
- **AND** Phoenix writes one root `.spo` file under the active `data` folder for every received section
- **AND** Phoenix rebuilds the section load/execution queues using the existing section pipeline

#### Scenario: Reconnect forces full section replacement
- **WHEN** Cacablu synchronizes an open project after Phoenix reconnects
- **THEN** Cacablu SHALL call the full section replacement operation even if Phoenix's section manifest appears equal
- **AND** Phoenix SHALL remove stale editor-published root `.spo` files
- **AND** Phoenix SHALL recreate runtime sections and root `.spo` files from every current enabled project bar

#### Scenario: Section sync follows asset sync
- **WHEN** a project synchronization requires both asset uploads and section replacement
- **THEN** Cacablu completes the asset and project-settings phases before requesting section replacement
- **AND** section scripts that reference published assets can resolve current files from Phoenix's active `data` folder

#### Scenario: Section synchronization reports the real section count
- **GIVEN** Cacablu sends 340 sections to Phoenix
- **WHEN** Phoenix reports separate internal writing and loading work for those sections
- **THEN** Cacablu displays synchronization progress with a total of 340
- **AND** the completed progress is `340/340`, not `680/680`

### Requirement: Bar to section serialization
Cacablu SHALL serialize enabled database bars into Phoenix-compatible section payloads and SHALL omit disabled database bars from Phoenix section payloads.

#### Scenario: Enabled bar is serialized
- **WHEN** Cacablu prepares an enabled bar for Phoenix
- **THEN** the payload includes the bar id, type, start time, end time, enabled flag, layer, source blend factor, destination blend factor, blend equation, and raw script text
- **AND** empty or whitespace-only bar type is treated as unconfigured and is not sent to Phoenix
- **AND** script line endings are normalized before transmission so Phoenix receives line-delimited section commands regardless of the SQLite text/blob newline style
- **AND** the canonical text is equivalent to a root `.spo` section with `:::<type>`, `id`, `start`, `end`, `enabled`, `layer`, `blend`, `blendequation`, a blank line, and the script body

#### Scenario: Disabled bar is omitted
- **WHEN** Cacablu prepares a disabled bar for Phoenix
- **THEN** Cacablu does not include that bar in replacement or update section payloads
- **AND** Cacablu does not report an unsupported-section error for that disabled bar

#### Scenario: Invalid enabled bar payload is rejected
- **WHEN** Cacablu sends a section replacement containing an invalid id, missing timing, invalid blend metadata, unsupported type, or malformed script for an enabled bar
- **THEN** Phoenix rejects the replacement with a structured error
- **AND** Phoenix does not partially replace the current runtime sections

### Requirement: Invalid bars do not abort project loading
Cacablu SHALL round section start/end times to three decimal places, validate each enabled project bar before sending a section batch, and isolate bar-level failures from valid sections.

#### Scenario: Section timing has more than three decimal places
- **WHEN** Cacablu serializes an enabled bar for Phoenix
- **THEN** Cacablu rounds its start and end times to three decimal places before transmission
- **AND** Phoenix receives the converted numeric times

#### Scenario: Legacy timing contains near-zero floating-point residue
- **WHEN** an enabled bar has a finite start or end time whose magnitude rounds to zero at three decimal places
- **THEN** Cacablu sends that time as zero
- **AND** the residue does not cause Phoenix to reject the section batch

#### Scenario: One enabled bar has timing Phoenix cannot represent
- **GIVEN** a project contains both valid enabled bars and an enabled bar whose timing remains non-finite or outside Phoenix's 32-bit floating-point range after conversion
- **WHEN** Cacablu synchronizes project sections
- **THEN** Cacablu omits only the invalid bar from the Phoenix request
- **AND** Cacablu sends every valid enabled supported bar
- **AND** project loading finishes
- **AND** Cacablu marks the invalid bar as a section error so the Timeline displays it in red

#### Scenario: Enabled bar has an invalid layer or time range
- **WHEN** an enabled bar has a non-integer or out-of-range 32-bit layer, or its end time is earlier than its start time
- **THEN** Cacablu does not send that bar to Phoenix
- **AND** Cacablu reports an issue associated with that bar id
- **AND** other valid bars continue synchronizing

#### Scenario: Every enabled bar is invalid
- **WHEN** no enabled project bar can be represented as a Phoenix section
- **THEN** section synchronization completes without aborting local project loading
- **AND** every invalid bar is marked as a section error

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
- **AND** the success response identifies only section id `17` as affected

#### Scenario: Single section update fails to load
- **WHEN** Phoenix receives a syntactically valid single-section update whose section cannot load
- **THEN** Phoenix returns a structured response that identifies section id `17` as failed
- **AND** the received section content remains available on disk for further editor correction

### Requirement: Disabled bar Phoenix removal
Cacablu SHALL remove disabled project bars from Phoenix runtime and disk state.

#### Scenario: Bar is disabled while Phoenix is connected
- **WHEN** the user toggles an enabled project bar to disabled while Phoenix is connected
- **THEN** Cacablu sends a Phoenix section delete request for that bar id
- **AND** Phoenix deletes the matching runtime section
- **AND** Phoenix deletes `<active-data-folder>/<id>.spo` for that bar id
- **AND** Phoenix leaves unrelated runtime sections and root `.spo` files intact

#### Scenario: Multiple bars are disabled while Phoenix is connected
- **WHEN** the user toggles multiple enabled project bars to disabled while Phoenix is connected
- **THEN** Cacablu sends a Phoenix section delete request containing all newly disabled bar ids
- **AND** Phoenix deletes only the matching runtime sections and root `.spo` files

#### Scenario: Bar is disabled while Phoenix is disconnected
- **WHEN** the user toggles an enabled project bar to disabled while Phoenix is disconnected
- **THEN** Cacablu persists the local disabled state
- **AND** Cacablu does not send a Phoenix request
- **AND** Cacablu does not create a disconnected-sync error event for the skipped delete

### Requirement: Re-enabled bar Phoenix publication
Cacablu SHALL publish re-enabled project bars to Phoenix as newly available sections.

#### Scenario: Bar is re-enabled while Phoenix is connected
- **WHEN** the user toggles a disabled project bar to enabled while Phoenix is connected
- **THEN** Cacablu sends that bar to Phoenix using the single-section update pathway
- **AND** Phoenix writes `<active-data-folder>/<id>.spo`
- **AND** Phoenix loads the matching runtime section
- **AND** Phoenix leaves unrelated runtime sections and root `.spo` files intact

#### Scenario: Re-enabled bar fails to load
- **WHEN** Phoenix rejects or fails to load a re-enabled bar
- **THEN** Cacablu keeps the local bar enabled
- **AND** Cacablu records the failure in the Events panel
- **AND** Cacablu marks the affected timeline bar as having a section sync error

#### Scenario: Bar is re-enabled while Phoenix is disconnected
- **WHEN** the user toggles a disabled project bar to enabled while Phoenix is disconnected
- **THEN** Cacablu persists the local enabled state
- **AND** Cacablu does not send a Phoenix request

### Requirement: Project-open sync excludes disabled bars
Cacablu SHALL exclude disabled project bars from initial project-open section synchronization.

#### Scenario: Project opens with disabled bars
- **WHEN** Cacablu opens a project containing disabled bars and Phoenix is connected
- **THEN** Cacablu compares Phoenix's section manifest against only enabled, supported project bars
- **AND** Cacablu's full replacement request omits disabled bars
- **AND** Phoenix removes any stale root `.spo` files whose ids correspond to omitted disabled bars during replacement cleanup

#### Scenario: Disabled bar has unsupported type
- **WHEN** Cacablu opens a project containing a disabled bar with an unsupported Phoenix section type
- **THEN** Cacablu does not send that bar to Phoenix
- **AND** Cacablu does not create an unsupported-section error event for that disabled bar

### Requirement: Section load order
Cacablu SHALL send enabled project bars to Phoenix in layer order.

#### Scenario: Sections are sent by layer
- **WHEN** Cacablu prepares a full project section replacement for Phoenix
- **THEN** enabled bars are ordered by ascending layer before they are sent
- **AND** bars within the same layer are ordered deterministically by start time, end time, and id

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
