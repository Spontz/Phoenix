## ADDED Requirements

### Requirement: Timeline project loading
Cacablu SHALL keep the timeline empty when no project is loaded and populate it from the loaded project database when a project is opened.

#### Scenario: No project is loaded
- **WHEN** the Timeline panel is opened without a loaded project
- **THEN** Cacablu shows an empty timeline state
- **AND** no placeholder bars or default layers are created

#### Scenario: Project is loaded
- **WHEN** Cacablu successfully opens a project
- **THEN** the Timeline panel shows one clip per project bar
- **AND** each clip uses the bar id, type, layer, start time, and end time from the project database

### Requirement: Timeline bar selection
Cacablu SHALL allow users to select bars from the timeline and expose the selected bar to other editor panels.

#### Scenario: Bar is selected
- **WHEN** the user single-clicks a timeline bar
- **THEN** Cacablu records that bar as the current selection
- **AND** the Bar Editor opens or reinitializes on the right side
- **AND** repeated single-clicks on the same bar still initialize the Bar Editor if needed

#### Scenario: Bar ids are displayed
- **WHEN** the user enables View > Display IDs
- **THEN** timeline bar labels show the bar id followed by one space and the bar name
- **AND** the menu item changes to `Ocultar IDs` until disabled

#### Scenario: Empty timeline space is selected
- **WHEN** the user clicks empty timeline space
- **THEN** Cacablu clears the bar selection
- **AND** selected-bar panels no longer show stale bar-specific state

### Requirement: Bar Editor bar properties
Cacablu SHALL provide a Bar Editor for selected timeline bars and persist applied bar property changes to the loaded project.

#### Scenario: Bar Editor opens for a selected bar
- **WHEN** the user single-clicks a timeline bar
- **THEN** Bar Editor shows Bar Type, Script Template, Save Template, script editor, Blend Source, Blend Destination, Blend Equation, and Apply controls

#### Scenario: Blend equation is edited
- **WHEN** the user opens the Blend Equation selector
- **THEN** Cacablu shows `Add`, `Subtract`, and `Reverse subtract`
- **AND** applied values are stored in the Phoenix-compatible section format

#### Scenario: Bar Editor changes are applied
- **WHEN** the user changes script or blend settings and selects Apply
- **THEN** Cacablu updates the selected bar in the active project session
- **AND** subsequent section synchronization uses the applied values

### Requirement: Timeline bar editing
Cacablu SHALL support creating, moving, resizing, deleting, and changing the layer of project bars from the timeline.

#### Scenario: Bar is created
- **WHEN** the user creates a bar on the timeline
- **THEN** Cacablu creates a corresponding project database bar with a stable id
- **AND** the new bar appears in the timeline at the requested layer and time range

#### Scenario: Bar is moved
- **WHEN** the user moves a timeline bar to another time or layer
- **THEN** Cacablu updates the bar start time, end time, and layer in the loaded project
- **AND** the bar remains selected after the move

#### Scenario: Bar move would overlap
- **WHEN** the user drags a selected bar onto another bar in the same layer
- **THEN** Cacablu blocks the overlapping position
- **AND** no overlapping bar edit is persisted

#### Scenario: Bar move is undone
- **WHEN** the user chooses Edit > Undo after a committed bar move
- **THEN** Cacablu pops the latest move action from the undo stack
- **AND** restores the bar's previous start time, end time, and layer when that position is valid

#### Scenario: Bar is resized
- **WHEN** the user resizes a timeline bar
- **THEN** Cacablu updates the bar start or end time in the loaded project
- **AND** the resulting duration remains positive

#### Scenario: Bar is deleted
- **WHEN** the user deletes a selected timeline bar
- **THEN** Cacablu removes that bar from the loaded project database
- **AND** the bar is removed from the timeline

### Requirement: Timeline edit validation
Cacablu SHALL validate timeline edits before committing them to the project database.

#### Scenario: Edit would create negative time
- **WHEN** a timeline edit would move a bar before time zero
- **THEN** Cacablu clamps or rejects the edit
- **AND** no negative start or end time is persisted

#### Scenario: Edit would create zero duration
- **WHEN** a timeline resize would produce zero or negative duration
- **THEN** Cacablu rejects or clamps the resize to the minimum valid duration
- **AND** the persisted bar keeps a positive duration

### Requirement: Timeline persistence
Cacablu SHALL persist committed timeline edits through the active project session.

#### Scenario: Project is saved after timeline edit
- **WHEN** the user saves a project after editing bars on the timeline
- **THEN** the saved SQLite project contains the edited bar values

#### Scenario: Timeline edit is cancelled before commit
- **WHEN** the user cancels an in-progress drag or resize interaction
- **THEN** Cacablu restores the bar to its pre-edit timeline position
- **AND** no cancelled values are persisted

### Requirement: Timeline-to-Phoenix section synchronization
Cacablu SHALL synchronize committed timeline bar changes to Phoenix using the existing project bar to section synchronization pathway.

#### Scenario: Timeline edit is committed while Phoenix is connected
- **WHEN** the user commits a timeline bar create, update, move, resize, layer change, or delete
- **THEN** Cacablu schedules a Phoenix section synchronization
- **AND** Phoenix receives the current project bar snapshot after debouncing intermediate timeline changes

#### Scenario: Bar move is synchronized to disk
- **WHEN** Phoenix receives a deferred single-section synchronization after a bar move
- **THEN** Phoenix rewrites only the `.spo` file for the moved bar id with updated timing and layer metadata
- **AND** Phoenix updates that runtime section without requiring a full section replacement

#### Scenario: Transport follows a bar move
- **WHEN** the user presses Play immediately after committing a bar move
- **THEN** Cacablu gives the transport command priority over the deferred section sync
- **AND** Play is not delayed by section replacement work

#### Scenario: Timeline edit is committed while Phoenix is disconnected
- **WHEN** the user commits a timeline bar edit and Phoenix is not connected
- **THEN** Cacablu keeps the local project edit
- **AND** Cacablu records an event indicating that Phoenix section sync could not be performed

#### Scenario: Project opens while Phoenix is disconnected
- **WHEN** the user opens a project and Phoenix is not connected
- **THEN** Cacablu skips initial Phoenix pool and section synchronization
- **AND** the project loads locally without attempting Phoenix fetch requests

#### Scenario: Phoenix rejects synchronized bars
- **WHEN** Phoenix returns section sync errors after a timeline edit
- **THEN** Cacablu records those errors in the Events panel
- **AND** Cacablu keeps the local timeline edit available for further correction

#### Scenario: Phoenix identifies failed sections
- **WHEN** Phoenix cannot load one or more synchronized sections
- **THEN** the section sync response includes failed section ids and messages
- **AND** Cacablu can associate those failures with timeline bars

#### Scenario: Section sync progress is displayed
- **WHEN** Cacablu can count local section preparation or manifest checking work
- **THEN** the sync modal advances its label and progress bar using actual processed counts
- **AND** one-shot Phoenix HTTP requests do not display stale partial counters or reset the progress bar to zero

#### Scenario: Timeline displays section errors
- **WHEN** Cacablu has error Events with known bar ids from section sync
- **THEN** matching timeline bars are colored red until those Events are cleared

### Requirement: Timeline playback controls
Cacablu SHALL keep timeline transport controls usable with the current Phoenix connection state.

#### Scenario: Phoenix is connected
- **WHEN** the user uses timeline transport controls
- **THEN** Cacablu sends the matching runtime command to Phoenix
- **AND** timeline playhead state follows Phoenix runtime state when available

#### Scenario: Phoenix is disconnected
- **WHEN** the user opens the Timeline panel while Phoenix is disconnected
- **THEN** Cacablu keeps the panel open
- **AND** transport actions that require Phoenix are disabled or produce a non-blocking disconnected state

#### Scenario: Playhead visual feedback
- **WHEN** playback is active
- **THEN** the playhead glow trail grows gradually as it advances
- **AND** when playback stops, the trail fades away gradually and only a subtle glow remains
