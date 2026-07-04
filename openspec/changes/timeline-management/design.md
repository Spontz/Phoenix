## Context

Cacablu currently renders project bars on a timeline and receives runtime playback state from Phoenix, but timeline editing is not yet the authoritative workflow for managing bars. Project bars live in the loaded SQLite project database and are synchronized to Phoenix as sections through the existing section sync API.

The timeline must remain usable when Phoenix is disconnected, and panels must be openable even without a project. Once a project is loaded, timeline edits should update the local project model first, then attempt Phoenix synchronization when the engine is connected. Errors should be observable in Events instead of blocking unrelated editing.

## Goals / Non-Goals

**Goals:**
- Make timeline bars editable from Cacablu.
- Persist timeline edits to the loaded project session.
- Keep timeline selection coordinated with inspector/events.
- Reuse the existing Phoenix section sync protocol after bar edits.
- Support disconnected editing and delayed/failed Phoenix sync reporting.
- Preserve Phoenix's section manifest and full replacement API as the source of section-load diagnostics for Cacablu.

**Non-Goals:**
- Introduce a new Phoenix section protocol unless the existing replacement API proves insufficient.
- Implement Phoenix-side timeline editing.
- Replace the existing project database schema unless an edit cannot be represented with current columns.
- Add advanced nonlinear editing features such as nested timelines, curve editing, or multi-user conflict resolution.

## Decisions

1. Cacablu timeline state is derived from the loaded project database, and edits write back through the project session.

   Rationale: the SQLite project remains the source of truth for bars. Keeping a separate timeline-only model would create sync ambiguity and make save behavior harder to reason about.

   Alternative considered: store edits only in timeline state until save. This was rejected because Phoenix sync needs the same data that will be persisted.

2. Timeline edits trigger debounced Phoenix section synchronization.

   Rationale: moving or resizing bars can generate many intermediate states. Debouncing avoids flooding Phoenix while keeping the engine close to the editor state.

   Alternative considered: sync on every pointer move. This would be simpler but too noisy and likely to interrupt preview/playback.

3. Sync failures are recorded in Events and do not discard local edits.

   Rationale: the user must be able to continue arranging a project even if Phoenix is disconnected or rejects a section. Events already provide a visible diagnostic surface.

   Alternative considered: rollback edits when Phoenix rejects them. This would protect runtime consistency but would make local editing fragile and frustrating.

4. Timeline selection uses the existing app selection state where practical.

   Rationale: inspector opening and selection-driven UI are already centralized. Extending that pattern avoids a parallel selection system.

5. Bar Editor is a Cacablu-side panel, not a Phoenix-side editor.

   Rationale: Phoenix already accepts complete section payloads and persists `.spo` files through the section sync API. Cacablu owns the right-side editor controls: Bar Type, Script Template, Save Template, script body, blend source, blend destination, blend equation, and Apply.

6. Sync progress must be truthful.

   Rationale: Cacablu can show real progress while preparing local section payloads and checking them against Phoenix's section manifest, but Phoenix's current `PUT /api/sections` is a single blocking request. Until Phoenix exposes streaming/progress events, Cacablu must not show stale `0/N` counters or reset the bar during that one-shot call.

7. Bar-level error styling is driven by section sync diagnostics.

   Rationale: Phoenix already returns failed section ids/messages. Cacablu uses those ids in Events and colors affected timeline bars red so the user can find invalid sections quickly.

## Risks / Trade-offs

- Local project state may temporarily differ from Phoenix runtime state -> show pending/failed sync events and retry on later edits or project reopen.
- Full section replacement after edits can be heavier than targeted section updates -> debounce edits and reuse exact-match checks before replacing.
- Drag/resize interactions can create invalid timing -> clamp durations and validate before persistence.
- Database write APIs may need focused extension -> keep changes scoped to bar CRUD/update helpers rather than broad DB refactors.
- One-shot section replacement cannot report per-section progress from Phoenix -> keep progress real on the Cacablu preparation side unless a future API adds progress events.

## Migration Plan

1. Add project-session helpers for bar create/update/delete.
2. Update the timeline panel to render empty without a project and editable clips with a project.
3. Persist timeline edits locally.
4. Add debounced Phoenix section sync after committed timeline changes.
5. Surface validation and sync failures in Events.

Rollback is limited to disabling the editable controls and retaining the existing read-only timeline render path.

## Open Questions

- Which exact gesture should create a new bar: double-click empty timeline space, context menu, toolbar action, or all of them?
- Should timeline edits auto-save the SQLite file immediately or mark the project dirty for explicit save?
- Should multi-select operations be included in the first implementation or follow as a separate change?
- Should Phoenix later expose section replacement progress over WebSocket, or is Cacablu-side preparation progress enough?
