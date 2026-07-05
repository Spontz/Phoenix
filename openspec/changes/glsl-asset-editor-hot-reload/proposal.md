## Why

Cacablu needs a shader editing workflow that can preview GLSL changes in Phoenix without committing them to the project database or to Phoenix's `data` folder. The same asset pipeline also needs to keep Phoenix sections honest when a published asset changes, disappears, or is unpublished.

## What Changes

- Add a Phoenix editor API operation for transient asset previews that updates runtime memory only and never writes disk.
- Extend persisted asset writes/deletes so Phoenix reloads or deactivates sections affected by the changed asset.
- Track which loaded sections depend on `pool` and `resources` assets so Phoenix can report the exact section IDs that were reloaded, deactivated, or failed.
- Make shader text assets, including `.glsl`, resolvable from an in-memory override while previewing.
- Return structured asset impact responses that Cacablu can show in Events without alerts.

## Capabilities

### New Capabilities

- `glsl-asset-editor-hot-reload`: Covers transient asset preview, persisted asset commits, dependent section reloads, and dependent section deactivation.

### Modified Capabilities

- Native editor API gains transient asset preview and asset-impact responses for persisted asset operations.

## Impact

- Phoenix editor API server and JSON request/response contracts.
- Asset path validation for `pool` and `resources`.
- Runtime asset resolution for shader/text assets.
- Section loading, dependency indexing, reload, and deactivation behavior.
- Existing persisted asset write/delete endpoints used by Cacablu.
