## ADDED Requirements

### Requirement: Demo Settings API

Phoenix SHALL expose editor API endpoints to read and replace demo control settings.

#### Scenario: Read demo settings

- **GIVEN** Phoenix is running
- **WHEN** Cacablu sends `GET /api/demo-settings`
- **THEN** Phoenix SHALL return current demo settings and supported log detail options.

#### Scenario: Replace demo settings

- **GIVEN** Phoenix receives a valid complete demo settings payload
- **WHEN** Cacablu sends `PUT /api/demo-settings`
- **THEN** Phoenix SHALL validate the payload, apply the settings in memory, persist `data/config/control.spo`, and return `ok: true`.

#### Scenario: Reject invalid demo settings

- **GIVEN** Phoenix receives an invalid demo settings payload
- **WHEN** the request is handled
- **THEN** Phoenix SHALL return `ok: false` with a structured error code
- **AND** Phoenix SHALL NOT replace `data/config/control.spo`.

### Requirement: control.spo Persistence

Phoenix SHALL persist accepted demo settings to `data/config/control.spo`.

#### Scenario: Write control file

- **GIVEN** Phoenix accepts demo settings
- **WHEN** persistence runs
- **THEN** Phoenix SHALL create `data/config` if needed
- **AND** Phoenix SHALL write `control.spo` with `demo_name`, `debug`, `debugEnableFloor`, `loop`, `sound`, `demo_start`, `demo_end`, `slave`, and `log_detail`.

#### Scenario: Persist calculated demo end

- **GIVEN** Cacablu sends `demoEnd` equal to the latest timeline bar end time
- **WHEN** Phoenix writes `control.spo`
- **THEN** Phoenix SHALL write that value as `demo_end`.

### Requirement: Log Detail Options

Phoenix SHALL expose only supported log detail values.

#### Scenario: Supported log detail levels

- **GIVEN** Cacablu requests demo settings
- **WHEN** Phoenix returns log detail options
- **THEN** the options SHALL be `0 None`, `1 Essential`, `2 Normal`, and `3 Verbose`.

#### Scenario: Legacy log detail value

- **GIVEN** a legacy `control.spo` contains `log_detail 4`
- **WHEN** Phoenix exposes settings through the editor API
- **THEN** Phoenix SHALL normalize the exposed value to `3 Verbose`
- **AND** new writes SHALL NOT emit `log_detail 4`.

### Requirement: Runtime Application

Phoenix SHALL apply accepted demo settings to the running engine state.

#### Scenario: Apply loop and sound controls

- **GIVEN** valid settings change `loop` or `sound`
- **WHEN** Phoenix accepts them
- **THEN** the running engine SHALL use the new loop and sound enabled state without Cacablu writing files directly.

#### Scenario: Apply debug floor and log detail

- **GIVEN** valid settings change `debugFloor` or `logDetail`
- **WHEN** Phoenix accepts them
- **THEN** Phoenix SHALL apply the debug floor flag and logger detail level to the running engine.
