## Context

Cacablu renders connection state as a pill-shaped `.connection-badge`. The badge receives status changes through shared application state, while Phoenix communication is split between a WebSocket connection controller and several small HTTP clients. There is currently no shared activity signal and no visual distinction between an idle live connection and recent traffic.

## Goals / Non-Goals

**Goals:**

- Give the connected badge a clear but unobtrusive pulsing green glow without a rotating decoration.
- Accelerate the glow pulse for a short, renewable interval whenever Cacablu sends or receives Phoenix traffic.
- Cover both WebSocket and HTTP communication through shared network-boundary helpers.
- Stop the animation outside the connected state and respect reduced-motion preferences.

**Non-Goals:**

- Measure bandwidth, request duration, latency, or connection quality.
- Persist activity history or expose network diagnostics.
- Change Phoenix protocols, endpoints, or connection semantics.

## Decisions

### Publish activity from Phoenix network boundaries

A small process-local activity module exposes a notification function, subscription, and a fetch wrapper. Phoenix HTTP clients use the wrapper, while the WebSocket controller notifies on valid incoming messages and successful sends. This keeps UI concerns out of protocol clients and avoids patching the browser's global `fetch` implementation.

Alternative considered: derive activity only from runtime-state updates. That would miss asset, section, settings, log, and WebRTC requests and would make activity dependent on one message type.

### Represent activity as a renewable faster pulse

The shell subscribes once and sets a temporary activity data attribute on the badge. Each notification renews a short timeout, so bursts remain fast without accumulating timers or counters. Connection status remains authoritative: CSS only pulses the glow when the badge is connected, and the shell clears activity immediately on any other status.

Alternative considered: store activity in global application state. Activity is transient presentation state with no consumer outside the badge, so adding it to persisted/shared snapshots would create unnecessary renders.

### Animate the existing badge glow

The existing badge markup remains accessible text. Connected state animates the badge's border and box shadows directly, producing a slow idle glow pulse without extra decorative DOM. Each pulse starts at full intensity and fades gradually. Activity selects a separate faster animation so entering the activity state produces an immediate flash instead of waiting for the idle cycle, like a storage activity light.

Under `prefers-reduced-motion: reduce`, pulsing is disabled. The static glow still communicates connection, and recent activity uses a temporary intensity change rather than motion.

## Risks / Trade-offs

- [Frequent runtime messages keep the indicator fast] → Use a short renewable window so speed reflects sustained traffic and returns to idle promptly.
- [Glow reduces text contrast] → Keep the pulse outside the text glyphs and retain the existing background/text colors.
- [Animation consumes resources] → Animate only the badge shadow, and disable it when disconnected or reduced motion is requested.
- [An HTTP client bypasses the shared wrapper] → Add focused search/test coverage for Phoenix client request boundaries.

## Migration Plan

Add the activity module and route existing Phoenix requests through it, then enhance the current badge without changing its label or status API. Rollback removes the subscription, wrapper calls, and connected pulse styles; no data migration is required.

## Open Questions

None.
