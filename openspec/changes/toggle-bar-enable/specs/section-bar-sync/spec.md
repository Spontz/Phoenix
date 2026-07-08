## MODIFIED Requirements

### Requirement: Bar to section serialization
Cacablu SHALL serialize enabled database bars into Phoenix-compatible section payloads and SHALL omit disabled database bars from Phoenix section payloads.

#### Scenario: Enabled bar is serialized
- **WHEN** Cacablu prepares an enabled bar for Phoenix
- **THEN** the payload includes the bar id, type, start time, end time, enabled flag, layer, source blend factor, destination blend factor, blend equation, and raw script text
- **AND** empty or whitespace-only bar type is treated as unconfigured and is not sent to Phoenix
- **AND** the canonical text is equivalent to a root `.spo` section with `:::<type>`, `id`, `start`, `end`, `enabled`, `layer`, `blend`, `blendequation`, a blank line, and the script body

#### Scenario: Disabled bar is omitted
- **WHEN** Cacablu prepares a disabled bar for Phoenix
- **THEN** Cacablu does not include that bar in replacement or update section payloads
- **AND** Cacablu does not report an unsupported-section error for that disabled bar

#### Scenario: Invalid enabled bar payload is rejected
- **WHEN** Cacablu sends a section replacement containing an invalid id, missing timing, invalid blend metadata, unsupported type, or malformed script for an enabled bar
- **THEN** Phoenix rejects the replacement with a structured error
- **AND** Phoenix does not partially replace the current runtime sections

## ADDED Requirements

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
