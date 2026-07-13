## MODIFIED Requirements

### Requirement: Timeline-to-Phoenix section synchronization
Cacablu SHALL synchronize committed timeline bar changes to Phoenix using the existing project bar to section synchronization pathway and SHALL automatically perform full project synchronization after reconnect.

#### Scenario: Timeline edit is committed while Phoenix is connected
- **WHEN** the user commits a timeline bar create, update, move, resize, layer change, or delete
- **THEN** Cacablu schedules a Phoenix section synchronization
- **AND** Phoenix receives the current project bar snapshot after debouncing intermediate timeline changes.

#### Scenario: Bar move is synchronized to disk
- **WHEN** Phoenix receives a deferred single-section synchronization after a bar move
- **THEN** Phoenix rewrites only the `.spo` file for the moved bar id with updated timing and layer metadata
- **AND** Phoenix updates that runtime section without requiring a full section replacement.

#### Scenario: Transport follows a bar move
- **WHEN** the user presses Play immediately after committing a bar move
- **THEN** Cacablu gives the transport command priority over the deferred section sync
- **AND** Play is not delayed by section replacement work.

#### Scenario: Timeline edit is committed while Phoenix is disconnected
- **WHEN** the user commits a timeline bar edit and Phoenix is not connected
- **THEN** Cacablu keeps the local project edit
- **AND** Cacablu does not send a Phoenix request.

#### Scenario: Project opens while Phoenix is disconnected
- **WHEN** the user opens a project and Phoenix is not connected
- **THEN** Cacablu skips initial Phoenix pool and section synchronization
- **AND** the project loads locally without attempting Phoenix fetch requests
- **AND** Cacablu does not create disconnected-sync error events for that skipped initial sync.

#### Scenario: Phoenix connects after project open
- **WHEN** Phoenix connects after Cacablu already has a project loaded
- **THEN** Cacablu automatically starts a forced full project synchronization
- **AND** Cacablu does not ask the user to confirm loading the project into Phoenix.

#### Scenario: Phoenix rejects synchronized bars
- **WHEN** Phoenix returns section sync errors after a timeline edit
- **THEN** Cacablu records those errors in the Events panel
- **AND** Cacablu keeps the local timeline edit available for further correction.

#### Scenario: Phoenix identifies failed sections
- **WHEN** Phoenix cannot load one or more synchronized sections
- **THEN** the section sync response includes failed section ids and messages
- **AND** Cacablu can associate those failures with timeline bars.

#### Scenario: Section sync progress is displayed
- **WHEN** Cacablu can count local section preparation or manifest checking work
- **THEN** the sync modal advances its label and progress bar using actual processed counts
- **AND** one-shot Phoenix HTTP requests do not display stale partial counters or reset the progress bar to zero.

#### Scenario: Timeline displays section errors
- **WHEN** Cacablu has tracked section error ids from section sync or asset impact responses
- **THEN** matching timeline bars are colored red until those ids are cleared by successful resync or project reset.

#### Scenario: Project open contains a malformed section
- **GIVEN** Phoenix is already connected
- **WHEN** the user opens a project containing a bar that cannot be represented by Phoenix
- **THEN** the project still opens locally
- **AND** valid bars and assets continue synchronizing
- **AND** the malformed bar is colored red in the Timeline.

### Requirement: New errors surface in Events without stealing focus
Cacablu SHALL make new error events visible through the workspace while preserving the user's active panel.

#### Scenario: New error arrives while Events is closed
- **GIVEN** the Events panel is not open
- **WHEN** Cacablu records a new event with error severity
- **THEN** Cacablu opens Events as an inactive tab
- **AND** the previously active panel remains active and focused.

#### Scenario: Non-error event arrives while Events is closed
- **WHEN** Cacablu records only information or warning events
- **THEN** Cacablu SHALL NOT automatically open Events.

#### Scenario: Events contains new errors
- **WHEN** the Events panel receives a new error event or Cacablu tracks a new section error id
- **THEN** its tab title displays a red dot
- **AND** the Events panel uses the custom notification-aware tab renderer

#### Scenario: User brings Events to the foreground
- **GIVEN** the Events tab displays its red error dot
- **WHEN** the user activates the Events panel
- **THEN** Cacablu marks its error notification as read
- **AND** the red dot disappears
- **AND** existing events and Timeline section-error highlighting remain unchanged.

#### Scenario: Error occurs while a project is opening
- **GIVEN** project loading has temporarily closed Timeline and Events
- **WHEN** synchronization records a new error
- **THEN** Cacablu retains a pending Events notification
- **AND** after Timeline is restored, Cacablu inserts Events as an inactive tab in Timeline's group
- **AND** the Events tab displays its red error dot.

#### Scenario: Section error exists without a new error event
- **WHEN** Cacablu adds a new section id to its tracked section errors
- **THEN** the Events notification opens using the same background behavior
- **AND** its tab title displays the red error dot.

#### Scenario: Local section validation fails
- **WHEN** Cacablu omits a malformed section before contacting Phoenix
- **THEN** it records the bar-specific issue as an error event
- **AND** the Events notification behavior applies to that issue.
