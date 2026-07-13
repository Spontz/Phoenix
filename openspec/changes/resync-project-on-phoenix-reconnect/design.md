## Context

Cacablu currently records project/Phoenix synchronization with a process-local `pending | synced` flag. Opening a project while Phoenix is connected runs pool, demo-settings, and section synchronization. A later disconnect does not invalidate `synced`, so the disconnected-to-connected callback neither resynchronizes nor prompts. Meanwhile Phoenix is a new process whose runtime sections and disk state may be stale.

Phoenix's existing APIs already support recursive `pool`/`resources` deletion, asset writes, configuration writes, and full section replacement. Cacablu must orchestrate those operations as one generation and must not let ordinary manifest shortcuts skip work after reconnect. Directory deletion must first deactivate dependent sections and release cached video ownership so Windows can remove open media files.

## Goals / Non-Goals

**Goals:**
- Automatically rebuild Phoenix from the currently open Cacablu project after every real reconnect.
- Compare editor-managed assets exactly, rebuilding them only when different, and force full section replacement.
- Restore assets before configuration and sections so section loading sees current dependencies.
- Coalesce duplicate connected notifications and prevent overlapping full syncs.
- Leave a failed or interrupted generation pending for retry.
- Freeze the Timeline transport while Phoenix is offline and preserve the active loop independently of the Timeline panel lifecycle.
- Keep project loading successful when individual bars cannot be represented by Phoenix, while highlighting those bars as errors and synchronizing every valid bar.
- Surface new errors without interrupting the current workspace by opening Events as an inactive tab and keeping a visible red error indicator in its title.

**Non-Goals:**
- Delete Phoenix bootstrap files that Cacablu cannot recreate, such as installation-owned loader/configuration templates.
- Synchronize while no Cacablu project is open.
- Block offline editing or roll back local edits after a Phoenix error.
- Add background incremental reconciliation while the connection remains continuously healthy.

## Decisions

### Treat each Phoenix process connection as a new remote generation

On a transition from any non-connected state to `connected`, Cacablu marks the project remote state pending and starts a full synchronization automatically. It does not trust the previous `synced` flag or ask for confirmation.

Alternative considered: resync only when Cacablu observed local edits while offline. This misses Phoenix restarts without local edits, where the new process still needs its runtime state rebuilt.

### Use one serialized synchronization coordinator

The shell owns a single in-flight project-sync promise/abort controller and a monotonically increasing generation. Duplicate connection notifications join or no-op against the active generation. A disconnect aborts the current generation and leaves the project pending. Completion marks synchronized only if the same project, connection generation, and session are still current.

Alternative considered: let the connection subscriber call each service independently. That permits overlapping recursive deletion and upload operations during connection flapping.

### Reuse exact asset reconciliation and force only runtime-bearing state

`syncPublishedPoolFilesToPhoenix` keeps its exact manifest comparison. On reconnect, identical `pool` contents are retained; any path, size, or hash difference clears the managed tree and republishes every enabled asset. `syncProjectBarsToPhoenix` gains a `forceReplace` option so reconnect always uses `replaceAll` without a manifest equality shortcut.

The ordered reconnect pipeline is:

1. Compare editor-managed assets and, only when different, clear/recreate their roots and upload every enabled project asset.
2. Reapply generated project configuration through the existing settings APIs.
3. Replace all runtime sections and root editor-owned `.spo` files.
4. Reapply the active loop interval when Cacablu has one selected.
5. Clear resolved sync errors and mark the generation synchronized.

Alternative considered: always delete the literal active `data` directory or `pool` subtree. Phoenix needs bootstrap/config files that are not all represented in the project database, while retransmitting an identical pool adds cost without rebuilding runtime state. “Full project resync” therefore reconciles assets exactly and always rebuilds configuration and sections.

### Report errors without accepting partial synchronization as complete

Asset failures, configuration failures, section failures, cancellation, or disconnect keep the generation pending. Existing Events and the synchronization modal report the failing phase. A later reconnect retries the complete sequence from its destructive first step.

Pool reconstruction verifies both destructive boundaries: after recursive deletion the Phoenix pool manifest must be empty, and after upload it must exactly match the current project snapshot. Each mismatch reports its first differing path and reason. This prevents a successful HTTP response from hiding a non-convergent disk state and avoids presenting skipped assets as an active copy phase when the initial manifest already matches.

Request-level failures remain fatal to the synchronization generation, but bar-level validation failures are collected before transmission. Cacablu first rounds section start/end times to three decimal places, then sends one `replaceAll` request containing only representable enabled bars, records an issue for every omitted bar, and allows project loading to finish. This prevents Phoenix's all-or-nothing request parser from rejecting valid sections alongside a malformed section. Rounded timing values must be finite and either zero or representable as a non-subnormal 32-bit float because Phoenix parses them with `std::stof`; layers must be 32-bit integers and time ranges must be ordered.

The timing conversion belongs to Cacablu's section serializer and does not require Phoenix parser changes. This also converts legacy floating-point residue near zero to `0.000`, matching the database's millisecond precision, while genuinely invalid values remain associated with red Timeline bars.

Phoenix's section manifest includes the existing runtime `Section::loaded` state for every entry. Cacablu may skip an ordinary project-open replacement only when serialized content matches and every corresponding manifest entry explicitly reports `loaded: true`. A matching entry with `loaded: false` is retried after asset synchronization, allowing repaired dependencies to recover; a missing flag from an older Phoenix build is treated as unknown and retried safely. The replacement response remains the source of detailed per-section failure messages, so Phoenix does not need a second persistent error store.

### Keep transport and loop state outside the Timeline panel lifecycle

When Phoenix disconnects, Timeline immediately sets its transport to not playing and stops local time extrapolation. The last displayed time remains frozen until Phoenix publishes runtime state again.

The selected loop interval is stored in shared `AppState`, not only inside the Timeline renderer closure. Reconnect synchronization reads that interval after section replacement and sends it through the existing runtime loop client. Every new Timeline panel instance hydrates its local loop from `AppState`, so closing and reopening the panel restores the indicator without changing Phoenix state.

Alternative considered: keep the loop only in the panel and query Phoenix when reopening. That loses the editor selection while Phoenix is offline and makes Phoenix, rather than Cacablu, the source of truth.

### Notify errors without stealing focus

`AppState` increments an error revision whenever one or more new error events are added. The shell compares revisions and newly tracked section-error ids. When Events is closed, it opens it with Dockview's native inactive-panel option. If an error arrives while a project is still opening and Timeline has temporarily been closed, the shell defers Events until Timeline is restored, then inserts Events inactive within Timeline's tab group. The user's active panel and keyboard focus remain unchanged. Events explicitly selects the registered `shell-tab` renderer (registering a `createTabComponent` factory alone does not make Dockview use it). That renderer displays a separate red dot while new errors remain unread. Activating Events marks the notification as read and removes the dot without deleting its events or clearing Timeline section errors. Section validation issues are added to Events as well as to the Timeline error-id set.

Alternative considered: activate Events for each error. This interrupts editing and can repeatedly move focus during a synchronization that emits several errors.

## Risks / Trade-offs

- [Large projects are expensive to compare after every engine restart] → Use the existing manifest/hash comparison and avoid transfer when exact content matches.
- [Connection flapping can interrupt destructive work] → Abort on disconnect and always restart the whole ordered generation after the next connection.
- [The phrase “entire data folder” includes Phoenix-owned bootstrap files] → Define and test explicit ownership boundaries; never remove files Cacablu cannot reconstruct.
- [A stale async completion could mark a newer session synchronized] → Compare session identity and connection generation before committing state.
- [Sections may briefly be absent while rebuilding] → Upload assets/config first and execute one `replaceAll` as the final mutation.
- [A loop can outlive its Timeline panel instance] → Keep the interval in shared application state and clear it only when the active project changes.

## Migration Plan

1. Strengthen exact asset comparison tests and add forced replacement to the section sync service.
2. Replace the prompt-based reconnect branch with the serialized coordinator.
3. Add reconnection, duplicate notification, disconnect-during-sync, and project-switch tests.
4. Add transport-disconnect and active-loop lifecycle coverage.
5. Extend section-manifest entries with runtime load state and release runtime video handles before deleting the managed pool.

Rollback consists of restoring the prompt-based connection branch and removing the forced options; no project database migration is required.

## Open Questions

None. Demo settings, graphics settings, sections, and the active loop are all included in the reconnect pipeline.
