## Context

The archived Timeline layer change introduced two overlapping models: session-added empty layers created by a command, and implicit presentation-only layers that continuously cover the Timeline and extend a full viewport below the last occupied layer. Once implicit layers were added, the explicit `New Layer` action stopped being necessary for creating or moving bars.

Cacablu currently carries the command through menu metadata, shell enablement and shortcut routing, a shared `TimelineLayerSession`, panel event handling, and tests. The implicit surface also consults that session when deciding which layer is last used.

## Goals / Non-Goals

**Goals:**

- Remove `Timeline > New Layer`, `Ctrl+L`, and the explicit layer event route.
- Remove session-only empty-layer state and derive the implicit surface from occupied database layers.
- Preserve a contiguous surface and one full viewport of unused layers below the last occupied layer.
- Preserve drag-to-create, scrolling, grid alignment, and all unrelated Timeline menu actions.
- Remove obsolete tests while retaining trusted-pointer coverage for the implicit workflow.

**Non-Goals:**

- Remove or rename the top-level `Timeline` menu.
- Change persisted bar layer numbers or the SQLite schema.
- Change Timeline bar creation, movement, playback, loops, or grid behavior.
- Modify Phoenix.

## Decisions

### Derive presentation layers directly from occupied bar layers

The Timeline panel will compute the last used layer from `db.bars` only, then materialize the same contiguous implicit surface and unused viewport beneath it. This eliminates `TimelineLayerSession` rather than retaining an abstraction whose only remaining job would be sorting bar layers.

Alternative considered: keep the session helper but remove only the menu. That leaves dead explicit-layer concepts and makes the surface harder to understand.

### Remove the command end to end

Menu metadata, shell command routing, shortcut interception, panel events, lifecycle cleanup, and explicit-layer unit tests will be removed together. `Ctrl+L` will no longer be intercepted by Cacablu and returns to the host/browser in every context.

Alternative considered: hide the menu action but keep the internal command. There is no remaining caller or product requirement for it.

### Keep implicit surface coverage as the regression boundary

The Playwright workflow will open a project, verify the continuous layer surface, create a bar with a trusted pointer drag in an unused visible row, confirm another full unused window appears below it, and verify scrolling and grid coverage. It will no longer assert menu or keyboard layer creation.

## Risks / Trade-offs

- [Removing explicit state shortens the surface after reopening] → The surface is intentionally anchored to occupied layers and always adds a full unused viewport.
- [A stale listener still intercepts Ctrl+L] → Remove shell and panel routing and verify the menu action and event identifiers are absent.
- [Test cleanup accidentally drops implicit-layer coverage] → Keep the real pointer, scroll, and grid assertions in the focused Playwright workflow.

## Migration Plan

1. Remove command metadata and shell routing.
2. Simplify Timeline panel construction to database-backed plus implicit layers.
3. Remove the explicit layer service and unit tests.
4. Rewrite Playwright coverage around the implicit surface only.
5. Validate Cacablu and the OpenSpec delta.

Rollback restores the command, session helper, and their tests. No stored data requires migration.

## Open Questions

None.
