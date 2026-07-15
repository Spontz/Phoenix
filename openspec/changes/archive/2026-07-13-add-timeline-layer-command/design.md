## Context

Cacablu currently labels its timeline action menu `Bars`, although the menu is becoming the home for commands that affect the wider Timeline. Timeline tracks are derived from the distinct numeric `layer` values present in project bars; the project schema has no independent layer rows, so an empty layer cannot currently survive a database-driven track rebuild.

The visible menu has already been renamed locally to `Timeline`. This change formalizes that behavior and adds `New Layer` through the same menu and shortcut routing used by existing application commands.

## Goals / Non-Goals

**Goals:**

- Formalize `Timeline` as the menu name without changing existing menu action IDs or shortcuts.
- Append one empty layer from `Timeline > New Layer` or `Ctrl+L`.
- Keep empty layers stable across Timeline rerenders for the active project session.
- Reuse existing numeric bar layers for persistence once an added layer contains a bar.
- Preserve text-editing keyboard behavior and avoid Phoenix or database schema changes.

**Non-Goals:**

- Persist a completely empty layer across project close/reopen.
- Add layer names, reordering, deletion, visibility, locking, or other track metadata.
- Change existing bar layer values when a new layer is appended.
- Add a Phoenix endpoint or runtime layer object.

## Decisions

### Treat empty layers as Timeline session presentation state

The Timeline panel will retain a set of explicitly added numeric layer IDs alongside layers derived from project bars. Track rebuilding will use the union of both sets, so database refreshes and ordinary rerenders do not remove an empty layer. The set is cleared when the project session closes or changes.

Once a bar is created or moved onto an added layer, its existing `bar.layer` field makes that occupied layer durable without a schema migration. An empty layer is intentionally session-only because neither Cacablu's SQLite model nor Phoenix needs an independent empty-layer entity.

Alternative considered: add a `LAYERS` table or reserved project variable. That would introduce project-format migration and metadata synchronization for a command whose only current requirement is an empty Timeline lane.

### Allocate the next layer after the current maximum

`New Layer` computes the maximum across database-backed tracks and session-added tracks, then appends `max + 1`; an empty Timeline starts at layer `0`. Repeated commands therefore produce deterministic unique IDs and do not renumber or move existing bars.

Alternative considered: fill numeric gaps. Reusing a gap can insert the new lane visually between existing layers rather than appending it, which conflicts with the command's name and expected placement.

### Route menu and shortcut through one application command

The menu action receives a stable internal ID such as `new-timeline-layer`. The shell uses one handler for the menu action and `Ctrl+L`, applies the same project/Timeline availability checks, and dispatches a focused Timeline command event. The mounted Timeline panel claims the event, updates its session-layer set, rebuilds tracks, and renders.

The shortcut handler ignores editable targets using Cacablu's existing text-target guard. It prevents default only when the application can execute the command. `Ctrl+L` is commonly reserved by browser chrome; the menu action remains authoritative on hosts where the browser does not deliver that chord to page content.

Alternative considered: implement separate menu and keyboard mutations inside the Timeline panel. That would duplicate enablement and behavior and make the shortcut dependent on panel-local focus.

### Materialize implicit layers across the visible Timeline surface

The Timeline will render contiguous 18-pixel lanes, including numeric gaps between database-backed layers. Below the last occupied or explicitly added layer it will keep at least one panel-height window of unused lanes. These surface lanes are presentation state: dragging on any one creates a bar with that row's numeric layer, after which the normal bar record makes it durable. They do not need to be added explicitly and do not change the database schema.

Implicit surface lanes do not count as used. `New Layer` still allocates after the last database-backed or session-added layer; that row may already be visible as part of the unused surface and becomes an explicit session layer.

A resize reconciliation expands the unused window when the panel becomes taller. It does not remove explicit or occupied layers when the panel shrinks. The integration test performs a trusted pointer drag near the bottom of the unused visible surface and verifies that the resulting bar uses that row and causes another full unused window to remain below it; merely asserting that lane DOM nodes exist is insufficient.

The creation preview rerenders the viewport during the pointer gesture. Pointer capture therefore remains on the stable Timeline panel root so the final `pointerup` survives that rerender and commits the bar.

The time grid is sized to the total rendered lane height rather than the viewport height. It therefore scrolls with the lane content and keeps its vertical guides visible and aligned throughout the full unused-layer window.

### Keep the rename structural rather than cosmetic

The menu union type, menu action definitions, menu order, and tests use `Timeline` directly instead of translating an internal `Bars` key only during rendering. Existing action IDs remain unchanged, so command handlers and stored behavior do not need migration.

## Risks / Trade-offs

- [A user expects an empty layer to survive reopening] → Make session-only behavior explicit; occupied layers persist naturally through bars, and independent layer persistence can be proposed later with real metadata requirements.
- [`Ctrl+L` is intercepted by browser chrome] → Keep `Timeline > New Layer` fully functional and test the application shortcut route where the host delivers the key event.
- [A database refresh removes an empty track] → Rebuild tracks from the union of bar layers and the session-added layer set.
- [Project switching leaks layers between projects] → Clear session-added layers whenever the Timeline observes a different or closed database session.

## Migration Plan

1. Retain the completed `Bars` to `Timeline` menu rename and update its regression coverage.
2. Add the New Layer action and shared command/shortcut route.
3. Add session-added layer state and union-based track rebuilding in the Timeline panel.
4. Verify project lifecycle, repeated commands, text contexts, and menu/shortcut parity.

Rollback removes the new menu action, shortcut handler, and session-added layer state. No stored project data requires migration or rollback.

## Open Questions

None. Persistence of metadata for completely empty layers remains outside this change.
