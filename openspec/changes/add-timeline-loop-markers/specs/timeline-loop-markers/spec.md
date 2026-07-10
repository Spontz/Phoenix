## ADDED Requirements

### Requirement: Marker Persistence
Cacablu SHALL persist timeline loop markers in a project database table named `MARKERS` with `id`, `time`, and `label` columns.

#### Scenario: Opening a project without markers table
- **WHEN** Cacablu opens an existing project database that does not contain `MARKERS`
- **THEN** Cacablu SHALL migrate the database by creating an empty `MARKERS` table
- **AND** the project SHALL open without changing existing bars, files, folders, FBOs, or variables.

#### Scenario: Opening a project with legacy lowercase markers table
- **WHEN** Cacablu opens an existing project database that contains `markers` instead of `MARKERS`
- **THEN** Cacablu SHALL migrate marker rows into `MARKERS`
- **AND** Cacablu SHALL use `MARKERS` for subsequent marker reads and writes.

#### Scenario: Saving marker rows
- **WHEN** the user creates, edits, drags, renames, or deletes a marker
- **THEN** Cacablu SHALL update the in-memory project model and the `MARKERS` table consistently
- **AND** saving the project SHALL persist the marker changes.

### Requirement: Timeline Ruler Loop Zones
Cacablu SHALL split the timeline time ruler into an upper seek zone and a lower loop-selection zone, while marker creation requires Shift+click on the ruler.

#### Scenario: Selecting a loop interval from the lower zone
- **GIVEN** the project has markers at `10` and `20`
- **WHEN** the user clicks the lower zone of the ruler at time `15`
- **THEN** Cacablu SHALL choose loop start `10` and loop end `20`
- **AND** Cacablu SHALL send that loop interval to Phoenix.

#### Scenario: Selecting a fallback loop interval
- **GIVEN** the demo start time is `0`, demo end time is `60`, and the project has one marker at `20`
- **WHEN** the user clicks the lower zone at time `5`
- **THEN** Cacablu SHALL choose loop start `0` and loop end `20`
- **WHEN** the user clicks the lower zone at time `30`
- **THEN** Cacablu SHALL choose loop start `20` and loop end `60`.

#### Scenario: Seeking outside the active loop
- **GIVEN** the active loop is `10` to `20`
- **WHEN** the user clicks the upper zone at time `5`
- **THEN** Cacablu SHALL set the demo time to `5`
- **AND** Cacablu SHALL NOT recompute or send a new active loop interval.

#### Scenario: Showing the active loop indicator
- **WHEN** Cacablu has an active loop interval
- **THEN** the timeline ruler SHALL show the loop interval indicator in the lower half of the ruler
- **AND** the upper half of the ruler SHALL NOT show a duplicate active loop indicator.

### Requirement: Marker Timeline Editing
Cacablu SHALL render timeline markers as inverted triangles on the ruler and allow marker creation with Shift+click plus selection, dragging, and deletion from marker handles.

#### Scenario: Creating a marker from the ruler
- **WHEN** the user Shift+clicks the ruler at a time that does not hit an existing marker
- **THEN** Cacablu SHALL create a marker at the clicked time
- **AND** the marker SHALL be shown as an inverted triangle on the ruler.

#### Scenario: Lower zone click does not create a marker
- **WHEN** the user clicks the lower zone of the ruler without Shift
- **THEN** Cacablu SHALL NOT create a marker
- **AND** Cacablu SHALL treat the click as a loop-selection request.

#### Scenario: Selecting a marker
- **WHEN** the user clicks an existing marker triangle
- **THEN** Cacablu SHALL select that marker
- **AND** the selected marker SHALL have a visible selected state.

#### Scenario: Opening marker in panel from the timeline
- **WHEN** the user double-clicks an existing marker triangle
- **THEN** Cacablu SHALL open or focus the Markers panel
- **AND** the panel SHALL select the double-clicked marker.

#### Scenario: Dragging a marker
- **GIVEN** a marker exists at time `10`
- **WHEN** the user drags its triangle to time `12`
- **THEN** Cacablu SHALL update the marker time to `12`
- **AND** the marker triangle SHALL move to the new time.

#### Scenario: Deleting a selected marker
- **GIVEN** a marker is selected
- **WHEN** the user presses Delete or Backspace
- **THEN** Cacablu SHALL delete the selected marker
- **AND** the marker triangle SHALL be removed from the timeline.

#### Scenario: Undoing marker deletion
- **GIVEN** the user deleted a selected marker
- **WHEN** the user invokes Undo
- **THEN** Cacablu SHALL restore the deleted marker with its original id, time, and label.

#### Scenario: Marker labels and vertical guides
- **WHEN** Cacablu renders timeline markers
- **THEN** each marker label SHALL appear to the right of its triangle when the marker has a non-empty label
- **AND** each marker SHALL render a vertical guide line through the timeline at the marker time.

### Requirement: Markers Panel
Cacablu SHALL provide a Markers panel from the Panels menu for viewing and editing marker labels and times.

#### Scenario: Opening the Markers panel
- **WHEN** the user chooses the Markers item from the Panels menu
- **THEN** Cacablu SHALL open a Markers panel
- **AND** the panel SHALL provide a quick search field
- **AND** the panel SHALL list project markers ordered by time in a listbox.

#### Scenario: Renaming a marker
- **GIVEN** a marker is listed in the Markers panel
- **WHEN** the user edits its label
- **THEN** Cacablu SHALL update the marker label in memory and in the `MARKERS` table
- **AND** the timeline marker SHALL continue to refer to the same marker id.

#### Scenario: Editing marker time from the panel
- **GIVEN** a marker is listed in the Markers panel
- **WHEN** the user edits its time to a valid numeric value
- **THEN** Cacablu SHALL update the marker time in memory and in the `MARKERS` table
- **AND** the timeline triangle SHALL move to the edited time.

### Requirement: Phoenix Active Loop Endpoint
Phoenix SHALL expose an editor HTTP endpoint that accepts an active loop interval from Cacablu and applies it to runtime playback.

#### Scenario: Applying an active loop
- **WHEN** Cacablu sends `PUT /api/runtime/loop` with `startTime` `10` and `endTime` `20`
- **THEN** Phoenix SHALL validate that both times are finite and that `endTime` is greater than `startTime`
- **AND** Phoenix SHALL apply the loop start time and loop end time through the runtime loop controls
- **AND** Phoenix SHALL return a success response containing the accepted start and end times.

#### Scenario: Rejecting an invalid loop
- **WHEN** Cacablu sends `PUT /api/runtime/loop` with an invalid interval
- **THEN** Phoenix SHALL return a `400 Bad Request` response
- **AND** Phoenix SHALL keep the previous active runtime loop unchanged.

#### Scenario: Looping during playback
- **GIVEN** Phoenix has accepted active loop start `10` and end `20`
- **WHEN** playback reaches or passes time `20`
- **THEN** Phoenix SHALL continue playback from time `10`
- **AND** playback SHALL remain inside the accepted loop interval until another active loop is applied.
