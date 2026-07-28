## ADDED Requirements

### Requirement: Automatic full project synchronization after Phoenix reconnect
Cacablu SHALL treat every transition from a non-connected Phoenix state to connected as a new remote generation and automatically republish the open project.

#### Scenario: Phoenix restarts while a project remains open
- **GIVEN** Cacablu has an open project that was previously synchronized
- **AND** Phoenix disconnects and later connects again
- **WHEN** Cacablu observes the new connected transition
- **THEN** Cacablu marks the previous remote state invalid
- **AND** Cacablu starts a forced full project synchronization without asking for confirmation.

#### Scenario: Project was edited while Phoenix was offline
- **GIVEN** Cacablu has an open project
- **AND** the user changed assets or bars while Phoenix was disconnected
- **WHEN** Phoenix reconnects
- **THEN** the forced synchronization uses the current project snapshot
- **AND** Phoenix receives every current section
- **AND** project assets are transferred only when their exact manifest differs.

#### Scenario: Phoenix connects without an open project
- **WHEN** Phoenix connects and Cacablu has no open project
- **THEN** Cacablu SHALL NOT delete or upload Phoenix data
- **AND** Cacablu SHALL NOT send a section replacement.

### Requirement: Ordered reconnect synchronization
Cacablu SHALL rebuild Phoenix in dependency order from the active project snapshot.

#### Scenario: Full reconnect synchronization succeeds
- **WHEN** Cacablu performs a reconnect synchronization
- **THEN** it first compares Cacablu-managed project assets and republishes them only when different
- **AND** it next reapplies generated project settings
- **AND** it then replaces every Phoenix runtime section from the current project bars
- **AND** it reapplies the selected active loop when one exists
- **AND** it marks the current connection generation synchronized only after all required phases complete.

#### Scenario: A synchronization phase fails
- **WHEN** any required reconnect synchronization phase fails or is cancelled
- **THEN** Cacablu keeps the Phoenix project state pending
- **AND** Cacablu records the failure through the existing synchronization progress and Events surfaces
- **AND** a later reconnect retries the complete ordered synchronization.

### Requirement: Serialized reconnect generations
Cacablu SHALL prevent overlapping destructive project synchronizations.

#### Scenario: Duplicate connected notifications arrive
- **WHEN** Cacablu receives multiple connected notifications for the same live Phoenix connection
- **THEN** at most one forced project synchronization SHALL run for that connection generation.

#### Scenario: Phoenix disconnects during synchronization
- **WHEN** Phoenix disconnects while a forced synchronization is running
- **THEN** Cacablu aborts the active generation when possible
- **AND** no late completion from that generation marks the project synchronized
- **AND** the next connected generation starts a new full synchronization.

#### Scenario: Project changes during synchronization
- **WHEN** the user opens a different project while a reconnect synchronization is in flight
- **THEN** Cacablu SHALL NOT mark the new project synchronized from the old project's completion
- **AND** synchronization of the new project uses its own current snapshot.

### Requirement: Runtime transport stops on disconnect
Cacablu SHALL stop extrapolating Phoenix playback when the engine disconnects.

#### Scenario: Phoenix stops during playback
- **GIVEN** the Cacablu timeline is following a playing Phoenix runtime
- **WHEN** Phoenix disconnects
- **THEN** Cacablu sets the timeline transport to not playing
- **AND** the displayed current time remains frozen until a new Phoenix runtime state or explicit connected transport action changes it.

### Requirement: Active loop survives Phoenix reconnect
Cacablu SHALL restore the currently selected timeline loop after Phoenix reconnects.

#### Scenario: Phoenix reconnects with an active loop selected
- **GIVEN** Cacablu has an active timeline loop interval
- **WHEN** Phoenix reconnect synchronization completes assets, settings, and section replacement
- **THEN** Cacablu sends the active loop interval to Phoenix
- **AND** Phoenix uses that start and end time until another loop is selected.

#### Scenario: Phoenix reconnects without an active loop
- **WHEN** Phoenix reconnects and Cacablu has no selected loop interval
- **THEN** Cacablu SHALL NOT send a runtime loop request.

#### Scenario: Timeline panel is reopened with an active loop
- **GIVEN** Cacablu has an active timeline loop interval
- **WHEN** the user closes and reopens the Timeline panel
- **THEN** the new Timeline panel instance restores the loop interval from shared application state
- **AND** the lower ruler shows the active loop indicator again.
