## Overview

Cacablu will expose `Edit > Demo Settings`. The panel edits project-level runtime controls and sends a normalized payload to Phoenix. Phoenix validates the payload, applies it to the running engine, and writes `data/config/control.spo`.

Phoenix already reads these variables through `SpoReader`:

```text
demo_name
debug
debugEnableFloor
loop
sound
demo_start
demo_end
slave
log_detail
```

The new API owns writing these values from Cacablu. Cacablu does not write Phoenix `data/config` directly.

## Log Detail Mapping

Current Phoenix code defines `LogLevel` as:

- `0`: None, no informational logging.
- `1`: Essential, maps to `LogLevel::high`.
- `2`: Normal, maps to `LogLevel::med`.
- `3`: Verbose, maps to `LogLevel::low`.

The example value `log_detail 4` is not valid in the current enum. `Logger::setLogLevel` ignores values above `3`. The new panel must not expose `4`; Phoenix may tolerate legacy `4` on read by normalizing it to `3` Verbose, but new writes use `0..3`.

## API

### Read Demo Settings

`GET /api/demo-settings`

Response:

```json
{
  "ok": true,
  "settings": {
    "demoName": "Phoenix demo engine",
    "loop": true,
    "sound": true,
    "debugFloor": true,
    "logDetail": 1,
    "demoStart": 0,
    "demoEnd": 50,
    "debug": true,
    "slave": true
  },
  "logDetailOptions": [
    { "label": "None", "value": 0 },
    { "label": "Essential", "value": 1 },
    { "label": "Normal", "value": 2 },
    { "label": "Verbose", "value": 3 }
  ]
}
```

### Replace Demo Settings

`PUT /api/demo-settings`

Request:

```json
{
  "requestId": "demo-settings-123",
  "demoName": "My demo",
  "loop": true,
  "sound": true,
  "debugFloor": true,
  "logDetail": 1,
  "demoEnd": 120
}
```

Phoenix derives fixed runtime values:

- `debug 1`
- `demo_start 0.0`
- `slave 1`

Response:

```json
{
  "requestId": "demo-settings-123",
  "ok": true,
  "settings": { "...": "..." },
  "path": "config/control.spo"
}
```

## Persistence Format

Phoenix writes `data/config/control.spo` using this order:

```text
demo_name <demoName>
debug 1
debugEnableFloor <0|1>
loop <0|1>
sound <0|1>
demo_start 0.0
demo_end <timeline end>
slave 1
log_detail <0|1|2|3>
```

`demo_end` comes from Cacablu because Cacablu owns the timeline database. It must be the maximum `endTime` of all bars. If no bars exist, it must be `0`.

## Validation

Phoenix rejects:

- Empty demo name after trimming.
- Negative or non-finite `demoEnd`.
- `logDetail` outside `0..3`.
- Invalid JSON or missing required fields.

Phoenix writes `control.spo` atomically. If persistence fails, it returns `ok: false` and leaves the prior runtime settings active.

## Runtime Apply

On accepted settings Phoenix updates:

- `m_demoName`
- `m_loop`
- `m_soundManager` or equivalent sound enabled flag exposed through existing control variables
- `m_debugEnableFloor`
- `m_demoStartTime`
- `m_demoEndTime`
- `m_slaveMode`
- `m_logLevel`

If any setting cannot be applied immediately, Phoenix still persists the file and returns a warning describing the restart requirement.
