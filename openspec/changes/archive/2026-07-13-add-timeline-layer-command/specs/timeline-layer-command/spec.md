## ADDED Requirements

### Requirement: Timeline commands use a Timeline menu
Cacablu SHALL expose timeline-specific actions under a top-level menu named `Timeline`.

#### Scenario: Timeline menu is rendered
- **WHEN** Cacablu renders its application menu bar
- **THEN** the former `Bars` menu is labeled `Timeline`
- **AND** its existing actions and shortcuts remain available under that menu.

### Requirement: User can append an empty timeline layer
Cacablu SHALL provide a `New Layer` action in the `Timeline` menu that appends one empty layer to the active Timeline.

#### Scenario: Add a layer to an existing timeline
- **GIVEN** a project and its Timeline panel are open
- **WHEN** the user invokes `Timeline > New Layer`
- **THEN** Cacablu appends one visible empty layer
- **AND** assigns it the next integer after the greatest layer currently represented by project bars or session-added layers.

#### Scenario: Add the first layer
- **GIVEN** the active project has no bars or visible layers
- **WHEN** the user invokes `New Layer`
- **THEN** Cacablu creates a visible empty layer numbered `0`.

#### Scenario: Add multiple layers
- **WHEN** the user invokes `New Layer` repeatedly
- **THEN** each invocation appends exactly one distinct layer
- **AND** the layer numbers increase monotonically without reusing an existing layer number.

#### Scenario: Project is unavailable
- **GIVEN** no project is open or no Timeline panel is available to receive the command
- **WHEN** Cacablu renders or invokes `New Layer`
- **THEN** the menu action is disabled or ignored
- **AND** no project or application state is mutated.

### Requirement: New Layer has a keyboard shortcut
Cacablu SHALL route `Ctrl+L` to the same New Layer command used by the Timeline menu when the application owns the keyboard context.

#### Scenario: Invoke New Layer from the keyboard
- **GIVEN** a project and its Timeline panel are open
- **AND** focus is not inside a text-editing control
- **WHEN** Cacablu receives `Ctrl+L`
- **THEN** it prevents the corresponding application-level default action
- **AND** appends exactly one layer using the same numbering behavior as the menu action.

#### Scenario: Shortcut occurs in a text editor
- **GIVEN** focus is inside Monaco, an input, textarea, select, or contenteditable element
- **WHEN** the user presses `Ctrl+L`
- **THEN** Cacablu leaves the shortcut to the focused editor or browser context
- **AND** does not append a timeline layer.

### Requirement: Empty layers follow the active project lifecycle
Cacablu SHALL retain explicitly added empty layers while the same project session remains active without adding a new SQLite schema concept.

#### Scenario: Timeline rerenders during the project session
- **GIVEN** an empty layer was added with `New Layer`
- **WHEN** the Timeline rerenders or reconciles tracks from updated project bars
- **THEN** the empty session layer remains visible.

#### Scenario: Bar is placed anywhere on the visible Timeline surface
- **GIVEN** a project Timeline contains visible vertical space below the ruler
- **WHEN** the user drags across an unoccupied horizontal interval in any visible row
- **THEN** Cacablu treats that row as a layer and creates the bar there using the dragged time interval
- **AND** the interaction does not require the layer to have been added explicitly or to contain an existing bar
- **AND** when the project is saved, the bar's numeric layer value persists through the current project database behavior
- **AND** reopening the project reconstructs that occupied layer from its bars.

#### Scenario: Unused layers remain available below content
- **GIVEN** the Timeline has a last occupied or explicitly added layer
- **WHEN** Cacablu lays out or resizes the Timeline panel
- **THEN** it provides at least one full visible window of unused layers below that layer
- **AND** creating a bar in that unused window moves the extension point so a further full window remains available below the new last used layer.

#### Scenario: Grid remains visible while scrolling layers
- **GIVEN** the Timeline contains enough layers to scroll vertically
- **WHEN** the user scrolls below the initially visible layers
- **THEN** the vertical time-grid lines continue across the visible layer surface
- **AND** remain aligned with the ruler and bars.

#### Scenario: Empty layer has no bars when project closes
- **GIVEN** a session-added layer remains empty
- **WHEN** the active project closes or is replaced
- **THEN** Cacablu discards that empty presentation-only layer
- **AND** does not alter the SQLite schema solely to persist it.
