## Why

The current Phoenix connection badge changes color but remains visually static, so it does not communicate the difference between a live connection and an idle status label. A subtle animated signal can make connection health and current Phoenix traffic immediately legible without opening another panel.

## What Changes

- Add a green glow to the connected Phoenix badge border.
- Pulse the connected green glow slowly while Phoenix is idle.
- Temporarily accelerate the glow pulses when Cacablu sends or receives Phoenix communication.
- Remove the connected animation immediately on disconnect or error.
- Respect reduced-motion preferences while preserving a static connected glow and activity emphasis.

## Capabilities

### New Capabilities

- `phoenix-connection-indicator`: Visual connected and communication-activity states for the Phoenix status badge.

### Modified Capabilities

None.

## Impact

- Cacablu connection badge markup and styles in `src/app/shell.ts` and `src/styles/app.css`.
- A small shared Phoenix activity signal used by WebSocket and HTTP client boundaries.
- Focused state and browser regression coverage.
- No Phoenix API or C++ changes.
