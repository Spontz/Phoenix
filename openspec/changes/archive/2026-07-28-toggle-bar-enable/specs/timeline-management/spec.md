## ADDED Requirements

### Requirement: Selected bar enable toggling
Cacablu SHALL allow users to toggle the enabled state of the currently selected timeline bars from the Bars menu.

#### Scenario: Toggle Enable is unavailable without selection
- **WHEN** no timeline bar is selected
- **THEN** the `Bars > Toggle Enable` menu item is disabled

#### Scenario: Toggle Enable is available for selected bars
- **WHEN** one or more timeline bars are selected
- **THEN** the `Bars > Toggle Enable` menu item is enabled
- **AND** the menu item shows shortcut `Ctrl+D`

#### Scenario: Selected bars are toggled from menu
- **WHEN** the user selects `Bars > Toggle Enable`
- **THEN** Cacablu inverts the `enabled` value of every selected project bar
- **AND** bars that were enabled become disabled
- **AND** bars that were disabled become enabled
- **AND** the selected bars remain selected

#### Scenario: Selected bars are toggled from keyboard
- **WHEN** the user presses `Ctrl+D` while one or more bars are selected and the focused element is not a text editor
- **THEN** Cacablu runs the same command as `Bars > Toggle Enable`
- **AND** the browser default action for that shortcut is prevented

#### Scenario: Toggle state is persisted
- **WHEN** the user toggles selected bars and saves the project
- **THEN** the saved SQLite project stores the updated `BARS.enabled` values

#### Scenario: Toggle Enable is undoable
- **WHEN** the user chooses Edit > Undo after toggling selected bars
- **THEN** Cacablu restores each affected bar to its previous `enabled` value
- **AND** the restored bars remain selected

#### Scenario: Disabled bars remain visible
- **WHEN** a selected bar is toggled from enabled to disabled
- **THEN** Cacablu keeps the bar visible in the timeline
- **AND** the bar is visually distinguishable from enabled bars
