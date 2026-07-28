## 1. Sound synchronization primitives

- [x] 1.1 Add per-sound locking for decoder/state access in `Sound`.
- [x] 1.2 Add a callback-facing synchronized mix/read method in `Sound` and keep existing seek semantics.

## 2. SoundManager callback and list safety

- [x] 2.1 Add `SoundManager` list synchronization for add/get/clear/stop/iteration paths.
- [x] 2.2 Update callback mixing flow to use safe sound snapshots and synchronized `Sound` decoding.

## 3. Verification

- [x] 3.1 Build the Phoenix Engine target after the threading changes.
- [x] 3.2 Perform a focused seek stress smoke check (repeated seek while playback is active) and verify no memory instability.
