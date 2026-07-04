## Why

Cacablu needs a first-class graphics configuration dialog instead of relying on manual or one-shot data folder generation. The editor must be able to send the rendering context and generic FBO table to Phoenix, and Phoenix must apply that configuration to the running engine and persist it as `data/config/graphics.spo`.

## What Changes

- Add a Phoenix editor API endpoint for reading and replacing the graphics configuration.
- Validate rendering context settings and the 25 generic Phoenix FBO entries before applying them.
- Apply accepted graphics settings to the in-memory window and FBO configuration, rebuilding generic FBOs when needed.
- Persist the accepted configuration to `data/config/graphics.spo` using the same variable names read by `SpoReader`.
- Return structured success, validation, apply, and write responses so Cacablu can show errors in Events without using alerts.

## Capabilities

### New Capabilities

- `graphics-settings-config`: Covers editor-driven graphics configuration, Phoenix runtime application, and `graphics.spo` persistence.

### Modified Capabilities

- Native editor API gains graphics configuration endpoints.

## Impact

- Phoenix editor API server and request routing.
- Graphics configuration model, validation, and serialization.
- Window rendering context state and generic FBO recreation.
- `data/config/graphics.spo` disk writes under the existing Phoenix data folder.
