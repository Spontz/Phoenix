## ADDED Requirements

### Requirement: Graphics Configuration API

Phoenix SHALL expose editor API endpoints that allow Cacablu to read the current graphics configuration and replace it with a complete new configuration.

#### Scenario: Read current graphics configuration

- **GIVEN** Phoenix is running with initialized graphics state
- **WHEN** Cacablu sends `GET /api/graphics`
- **THEN** Phoenix SHALL return the normalized rendering context and 25 generic FBO rows.

#### Scenario: Replace graphics configuration

- **GIVEN** Phoenix is running and receives a valid complete graphics payload
- **WHEN** Cacablu sends `PUT /api/graphics`
- **THEN** Phoenix SHALL validate the payload, apply it in memory, persist `data/config/graphics.spo`, and return `ok: true`.

#### Scenario: Reject invalid graphics configuration

- **GIVEN** Phoenix receives a graphics payload with invalid context or FBO data
- **WHEN** Phoenix handles the request
- **THEN** Phoenix SHALL return `ok: false` with code `invalid-graphics-config`
- **AND** Phoenix SHALL NOT modify the active runtime configuration
- **AND** Phoenix SHALL NOT replace `data/config/graphics.spo`.

### Requirement: Rendering Context Settings

Phoenix SHALL support editor-provided rendering context settings for color depth, width, height, fullscreen, V-sync, and target FPS.

#### Scenario: Apply immediate context settings

- **GIVEN** a valid graphics payload changes width, height, or V-sync
- **WHEN** Phoenix accepts the payload
- **THEN** Phoenix SHALL update the in-memory window properties, recalculate aspect ratio, and apply the setting through the existing runtime path.

#### Scenario: Context value requires restart

- **GIVEN** a valid graphics payload contains a setting that cannot be safely applied immediately
- **WHEN** Phoenix accepts and persists the payload
- **THEN** Phoenix SHALL include a restart-required warning in the success response.

### Requirement: Generic FBO Table

Phoenix SHALL accept exactly 25 generic FBO entries that map to `Window::fboConfig[0..24]`.

#### Scenario: Configure ratio-based FBOs

- **GIVEN** FBO rows `0` through `19` contain positive ratio values, valid formats, attachment counts, and filter values
- **WHEN** Phoenix applies the graphics payload
- **THEN** Phoenix SHALL update those FBO configs as ratio-based FBOs derived from the current window size.

#### Scenario: Configure explicit-size FBOs

- **GIVEN** FBO rows `20` through `24` contain positive width and height values, valid formats, attachment counts, and filter values
- **WHEN** Phoenix applies the graphics payload
- **THEN** Phoenix SHALL update those FBO configs as explicit-size FBOs.

#### Scenario: Rebuild generic FBOs

- **GIVEN** Phoenix accepts a graphics payload that changes any generic FBO setting
- **WHEN** runtime apply runs
- **THEN** Phoenix SHALL recreate the generic FBOs
- **AND** Phoenix SHALL NOT alter effect-specific or preview-capture FBO ownership.

### Requirement: graphics.spo Persistence

Phoenix SHALL persist accepted graphics configurations to `data/config/graphics.spo`.

#### Scenario: Write accepted graphics file

- **GIVEN** Phoenix accepts and applies a graphics configuration
- **WHEN** persistence runs
- **THEN** Phoenix SHALL create `data/config` if it does not exist
- **AND** Phoenix SHALL write `graphics.spo` using the variable names consumed by `SpoReader`.

#### Scenario: Persistence failure

- **GIVEN** Phoenix cannot write `data/config/graphics.spo`
- **WHEN** it handles a graphics replacement request
- **THEN** Phoenix SHALL return `ok: false` with code `graphics-write-failed`
- **AND** Phoenix SHALL keep the previous runtime configuration active.
