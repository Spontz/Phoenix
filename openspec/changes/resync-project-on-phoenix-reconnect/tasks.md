## 1. Forced Synchronization Services

- [x] 1.1 Strengthen Cacablu project asset synchronization to compare paths, sizes, and hashes exactly, retaining identical `pool` content and rebuilding it only when different.
- [x] 1.2 Add a `forceReplace` option to project section synchronization that skips manifest equality and always calls Phoenix `replaceAll` with the current enabled project bars.
- [x] 1.3 Ensure reconnect asset synchronization reports scanning/cleaning/copying progress accurately and propagates cancellation or connection failures.
- [x] 1.4 Add unit tests proving reconnect asset sync skips an exact match, rebuilds differing content, and clears stale Phoenix assets for an empty project.
- [x] 1.5 Add unit tests proving forced section sync calls `replaceAll` even when manifests match and includes every current enabled supported bar.
- [x] 1.6 Round section timing to three decimal places in Cacablu, validate enabled bars against Phoenix's numeric constraints, omit malformed bars from batch requests, and report bar-specific issues.
- [x] 1.7 Ensure all-invalid section snapshots do not abort local project loading and add regression coverage for mixed valid/invalid and all-invalid projects.
- [x] 1.8 Short-circuit exact pool matches with `0 copied, N skipped` and report the first manifest difference that requires rebuilding.
- [x] 1.9 Verify pool emptiness after cleanup and exact manifest convergence after upload; fail the sync generation when either check fails.
- [x] 1.10 Add regression coverage for exact-match progress, stale files surviving cleanup, and a non-matching rebuilt manifest.
- [x] 1.11 Expose Phoenix runtime load state in section manifest entries and require explicit `loaded: true` before Cacablu skips an ordinary section replacement.
- [x] 1.12 Add regression coverage for matching loaded sections, matching failed sections, and legacy manifests without runtime load state.

## 2. Reconnect Synchronization Coordinator

- [x] 2.1 Replace the `pending | synced` flag and reconnect confirmation prompt in Cacablu shell with connection-generation-aware synchronization state.
- [x] 2.2 Detect every real non-connected-to-connected transition, invalidate prior Phoenix state, and automatically start forced synchronization when a project is open.
- [x] 2.3 Serialize full project synchronization so duplicate connected notifications cannot start overlapping delete/upload/replace operations.
- [x] 2.4 Abort or invalidate an in-flight generation when Phoenix disconnects or the active project session changes.
- [x] 2.5 Run reconnect phases in order: exact asset reconciliation, generated project settings, then forced full sections.
- [x] 2.6 Mark a generation synchronized only when the same connection and project session complete every required phase; otherwise leave it pending for retry.
- [x] 2.7 Preserve existing Events and progress-modal reporting and remove the obsolete reconnect confirmation state/UI.

## 3. Ownership and Phoenix API Coverage

- [x] 3.1 Document in code the Cacablu-managed `data` roots versus Phoenix-owned bootstrap files and verify current scoped asset/section APIs remove all stale managed content.
- [x] 3.2 Reuse the existing scoped Phoenix directory API without broadening its active-data-root confinement or path traversal surface.
- [x] 3.3 Ensure full section replacement removes stale editor-published root `.spo` files before recreating the current project snapshot.
- [x] 3.4 Deactivate dependent sections and release cached video handles before Phoenix recursively deletes the managed pool on Windows.

## 4. Integration Verification

- [x] 4.1 Add shell/integration coverage for open project → connected → disconnected edits → reconnect → automatic full asset and section synchronization.
- [x] 4.2 Add coverage that connecting without a project performs no destructive Phoenix operations.
- [x] 4.3 Add coverage for duplicate connected notifications, disconnect during synchronization, and switching projects during synchronization.
- [x] 4.4 Verify a failed reconnect generation remains pending and a later reconnect retries the complete sequence.
- [x] 4.5 Run Cacablu typecheck/unit tests and rebuild Phoenix after the directory-delete lifecycle correction.
- [x] 4.6 Stop Timeline playback extrapolation immediately when Phoenix disconnects and verify the current time remains frozen.
- [x] 4.7 Store the selected loop in shared Cacablu state and restore it in Phoenix after reconnect section synchronization.
- [x] 4.8 Hydrate each Timeline panel instance from the shared active loop so closing and reopening the panel preserves its indicator.
- [x] 4.9 Verify the attached project's near-zero `startTime` is converted to zero in Cacablu, both sections synchronize, and project loading completes.
- [x] 4.10 Track newly added error events independently from warnings/info and automatically open Events as an inactive Dockview tab without changing focus.
- [x] 4.11 Add a red error dot to the Events tab title, record local section validation issues as error events, and add focused state/UI regression coverage.
- [x] 4.12 Defer error-triggered Events creation during project opening until Timeline is restored, include newly tracked section-error ids, and verify the real load ordering.
- [x] 4.13 Explicitly select the notification-aware Dockview tab component for Events and verify the rendered red indicator in a browser.
- [x] 4.14 Track the red Events indicator as an unread-error notification and clear it when the Events panel becomes active without clearing error data.
