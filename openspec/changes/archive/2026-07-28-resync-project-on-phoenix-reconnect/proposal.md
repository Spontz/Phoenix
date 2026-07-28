## Why

When Phoenix stops while a Cacablu project remains open, edits continue only in Cacablu. After Phoenix reconnects, Cacablu currently keeps the project marked as synchronized and does not republish it, so Phoenix starts with stale files and sections from its local `data` folder.

## What Changes

- Treat every Phoenix disconnected-to-connected transition as invalidating the previous remote project state.
- Automatically perform a forced full project synchronization when Phoenix reconnects and Cacablu has a project open; do not require a confirmation prompt.
- Reconcile Phoenix's editor-managed assets with the active Cacablu project, skipping `pool` transfer when its exact manifest matches and rebuilding it when any path, size, or hash differs.
- Reapply generated project configuration and replace all Phoenix runtime sections and their root `.spo` files on every reconnect.
- Serialize reconnect synchronization so duplicate connection notifications cannot start overlapping destructive resets.
- Keep the project locally editable while Phoenix is offline and retry a failed/interrupted full synchronization on the next reconnect.
- Stop Timeline playback extrapolation and freeze the displayed time as soon as Phoenix disconnects.
- Store the selected loop in shared Cacablu state, restore it in Phoenix after reconnect synchronization, and preserve its Timeline indicator when the panel is closed and reopened.
- Round section start/end times to three decimal places in Cacablu and validate payloads independently before transmission so one malformed project bar cannot abort project loading; send the valid sections and mark rejected bars as Timeline errors.
- Require Phoenix's section manifest to report whether each persisted section loaded successfully, and retry matching sections whose runtime state is failed or unknown.
- Open the Events tab in the background when a new error arrives and show a red dot in its title until the user brings Events to the foreground.

## Capabilities

### New Capabilities
- `phoenix-reconnect-sync`: Detects a real Phoenix reconnection and orchestrates one automatic, ordered, full project republish from Cacablu.

### Modified Capabilities
- `data-asset-sync`: Requires reconnect synchronization to compare exact asset manifests and clear/republish managed assets only when they differ.
- `section-bar-sync`: Requires reconnect synchronization to perform a full section replacement after assets and configuration are restored, even if the manifest appears to match.
- `timeline-management`: Replaces the reconnect confirmation prompt with automatic full project synchronization whenever a project is already open.

## Impact

- Cacablu connection transition handling and project synchronization state in `src/app/shell.ts`.
- Cacablu pool/asset and section synchronization services and their unit tests.
- Cacablu section serialization applies three-decimal timing precision and mirrors Phoenix's finite 32-bit floating-point timing and integer layer limits before building a batch request.
- Cacablu shared application state for the active loop and Timeline panel hydration.
- Cacablu workspace tab activation and Events error notification state.
- Phoenix adds a `loaded` flag to existing section-manifest entries; its directory-delete implementation releases dependent runtime video handles before removing `pool`.
- Cacablu's open project is the source of truth, while identical `pool` content is retained to avoid unnecessary transfer.
