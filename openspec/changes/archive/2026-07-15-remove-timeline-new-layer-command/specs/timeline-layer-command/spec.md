## ADDED Requirements

### Requirement: Implicit layers cover the active Timeline surface
Cacablu SHALL materialize contiguous implicit layers across the active Timeline without requiring a separate layer-creation command or stored empty-layer entity.

#### Scenario: Bar is placed anywhere on the visible Timeline surface
- **GIVEN** a project Timeline contains visible vertical space below the ruler
- **WHEN** the user drags across an unoccupied horizontal interval in any visible row
- **THEN** Cacablu treats that row as a layer and creates the bar there using the dragged time interval
- **AND** the interaction does not require that layer to contain an existing bar
- **AND** saving and reopening the project reconstructs the occupied layer from the bar's numeric layer value.

#### Scenario: Unused layers remain available below content
- **GIVEN** the Timeline has a last occupied layer
- **WHEN** Cacablu lays out or resizes the Timeline panel
- **THEN** it provides at least one full visible window of unused layers below that layer
- **AND** creating a bar in that unused window moves the extension point so a further full window remains available below the new last occupied layer.

#### Scenario: Grid remains visible while scrolling layers
- **GIVEN** the Timeline contains enough layers to scroll vertically
- **WHEN** the user scrolls below the initially visible layers
- **THEN** the vertical time-grid lines continue across the visible layer surface
- **AND** remain aligned with the ruler and bars.

## REMOVED Requirements

### Requirement: User can append an empty timeline layer
**Reason**: The continuous implicit layer surface already makes every visible row available for bar creation, so an explicit empty-layer command is redundant.

**Migration**: Create a bar by dragging across any unused visible Timeline row; scroll downward to access a full additional window of implicit layers.

### Requirement: New Layer has a keyboard shortcut
**Reason**: The removed `New Layer` command no longer has an application action to invoke.

**Migration**: `Ctrl+L` returns to the host/browser. Use direct drag-to-create on the implicit Timeline surface.

### Requirement: Empty layers follow the active project lifecycle
**Reason**: Session-added empty layers are removed. Empty rows are now derived presentation-only layers, while occupied layers persist through bars.

**Migration**: No stored project migration is required. Occupied layers continue to reconstruct from existing bar layer values.
