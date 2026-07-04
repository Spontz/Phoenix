## Why

Cacablu needs first-class timeline editing so project bars can be created, selected, arranged, and inspected from the timeline instead of being treated as a mostly read-only visualization. This is the next step after project asset and section synchronization because timeline edits must become the source of the bars sent to Phoenix.

## What Changes

- Add timeline management for project bars, including selection, creation, deletion, movement, resize, and layer assignment.
- Keep the timeline empty when no project is loaded and populate it from the opened project database.
- Persist timeline edits back to the loaded project database.
- Reflect timeline selection in the inspector and other relevant panels.
- Synchronize changed bars to Phoenix using the existing section sync pathway.
- Surface validation and Phoenix sync errors through the Events panel without blocking general editing.
- Provide enough section sync response detail for Cacablu to mark failed timeline bars and avoid pretending that one-shot engine requests have partial progress.

## Capabilities

### New Capabilities
- `timeline-management`: Covers editable Cacablu timeline bars, persistence to the project database, selection/inspection, and Phoenix section synchronization triggered by timeline edits.

### Modified Capabilities

## Impact

- Cacablu timeline panel, timeline state model, project database session, inspector selection state, Events panel integration, and Phoenix section sync client.
- Phoenix section sync API behavior is reused; no new Phoenix runtime section protocol is expected for this change unless timeline edits expose a missing contract.
- Cacablu owns Bar Editor UI, real local sync progress, compact Events rendering, red error styling, and playhead glow behavior.
