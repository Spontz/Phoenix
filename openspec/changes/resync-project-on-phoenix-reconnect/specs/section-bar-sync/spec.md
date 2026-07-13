## MODIFIED Requirements

### Requirement: Initial project bar synchronization
Cacablu SHALL synchronize project database bars to Phoenix runtime sections when a project opens and SHALL force full replacement after Phoenix reconnects.

#### Scenario: Project bars already match Phoenix sections during ordinary project open
- **WHEN** Cacablu opens a project outside reconnect recovery and the serialized project bars exactly match Phoenix's section manifest
- **AND** every matching Phoenix manifest entry reports that its runtime section loaded successfully
- **THEN** Cacablu does not ask Phoenix to delete or recreate sections
- **AND** Phoenix keeps the existing runtime sections.

#### Scenario: Matching Phoenix section has a runtime load error
- **WHEN** a serialized project bar matches Phoenix's persisted section metadata and content
- **AND** Phoenix's manifest reports that the runtime section did not load successfully
- **THEN** Cacablu sends a full section replacement after project assets and settings are synchronized
- **AND** Cacablu uses the replacement response to associate any repeated load failure with the matching Timeline bar.

#### Scenario: Phoenix does not report section runtime state
- **WHEN** a serialized project bar matches a section manifest entry from an older Phoenix version without runtime load state
- **THEN** Cacablu treats that section state as unknown
- **AND** Cacablu sends a full section replacement instead of assuming the section is healthy.

#### Scenario: Project bars differ from Phoenix sections
- **WHEN** Cacablu opens a project and Phoenix has any missing, extra, or changed section relative to the serialized project bars
- **THEN** Cacablu sends a full section replacement request to Phoenix
- **AND** Phoenix deletes all current runtime sections
- **AND** Phoenix creates runtime sections for every project bar in the request
- **AND** Phoenix writes one root `.spo` file under the active `data` folder for every received section
- **AND** Phoenix rebuilds the section load/execution queues using the existing section pipeline.

#### Scenario: Reconnect forces full section replacement
- **WHEN** Cacablu synchronizes an open project after Phoenix reconnects
- **THEN** Cacablu SHALL call the full section replacement operation even if Phoenix's section manifest appears equal
- **AND** Phoenix SHALL remove stale editor-published root `.spo` files
- **AND** Phoenix SHALL recreate runtime sections and root `.spo` files from every current enabled project bar.

#### Scenario: Section sync follows asset sync
- **WHEN** a project synchronization requires both asset uploads and section replacement
- **THEN** Cacablu completes the asset and project-settings phases before requesting section replacement
- **AND** section scripts that reference published assets can resolve current files from Phoenix's active `data` folder.

### Requirement: Invalid bars do not abort project loading
Cacablu SHALL round section start/end times to three decimal places, validate each enabled project bar before sending a section batch, and isolate bar-level failures from valid sections.

#### Scenario: Section timing has more than three decimal places
- **WHEN** Cacablu serializes an enabled bar for Phoenix
- **THEN** Cacablu rounds its start and end times to three decimal places before transmission
- **AND** Phoenix receives the converted numeric times.

#### Scenario: Legacy timing contains near-zero floating-point residue
- **WHEN** an enabled bar has a finite start or end time whose magnitude rounds to zero at three decimal places
- **THEN** Cacablu sends that time as zero
- **AND** the residue does not cause Phoenix to reject the section batch.

#### Scenario: One enabled bar has timing Phoenix cannot represent
- **GIVEN** a project contains both valid enabled bars and an enabled bar whose timing remains non-finite or outside Phoenix's 32-bit floating-point range after conversion
- **WHEN** Cacablu synchronizes project sections
- **THEN** Cacablu omits only the invalid bar from the Phoenix request
- **AND** Cacablu sends every valid enabled supported bar
- **AND** project loading finishes
- **AND** Cacablu marks the invalid bar as a section error so the Timeline displays it in red.

#### Scenario: Enabled bar has an invalid layer or time range
- **WHEN** an enabled bar has a non-integer or out-of-range 32-bit layer, or its end time is earlier than its start time
- **THEN** Cacablu does not send that bar to Phoenix
- **AND** Cacablu reports an issue associated with that bar id
- **AND** other valid bars continue synchronizing.

#### Scenario: Every enabled bar is invalid
- **WHEN** no enabled project bar can be represented as a Phoenix section
- **THEN** section synchronization completes without aborting local project loading
- **AND** every invalid bar is marked as a section error.
