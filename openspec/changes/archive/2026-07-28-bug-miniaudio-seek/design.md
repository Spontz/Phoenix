## Context

Phoenix mixes audio on miniaudio's callback thread while timeline and section logic can call `seekSound()` on the main thread. The current `Sound` implementation exposes raw decoder access without synchronization, and `SoundManager` iterates the shared sound list in the callback while other paths can mutate it.

This creates data races around:
- Decoder state (`ma_decoder_read_pcm_frames` vs `ma_decoder_seek_to_pcm_frame` and unload).
- Playback status reads/writes across threads.
- Shared sound-list iteration vs clear/reload operations.

## Goals / Non-Goals

**Goals:**
- Make decoder access thread-safe for read/seek/load/unload paths.
- Remove sound-list mutation races with the callback thread.
- Preserve existing runtime behavior and API shape used by sections.

**Non-Goals:**
- Reworking the entire audio architecture or replacing miniaudio.
- Changing timeline semantics or section playback policy.
- Adding new external dependencies.

## Decisions

1. Add per-`Sound` mutex protection around decoder and status state.
   - Why: This is the smallest reliable boundary where read/seek/unload contention happens.
   - Alternative considered: Locking only `seekSound()`; rejected because callback reads and load/unload remain unsafe.

2. Move callback mixing through a synchronized `Sound` method instead of using raw `ma_decoder*`.
   - Why: Prevents use-after-free and read/seek races by guaranteeing decoder use under the same lock.
   - Alternative considered: Returning locked decoder handles; rejected because lock lifetime would be hard to enforce safely.

3. Add `SoundManager` list mutex and snapshot-copy shared pointers before callback iteration.
   - Why: Avoids holding list lock during decoder reads while preventing iterator invalidation when sounds are cleared/reloaded.
   - Alternative considered: Hold the list lock for full callback mixing; rejected to avoid long lock contention and callback stalls.

## Risks / Trade-offs

- [Risk] More lock operations in audio paths can add overhead.  
  → Mitigation: Keep lock scope narrow and use list snapshotting to avoid coarse-grained contention.

- [Risk] Lock ordering mistakes could deadlock.  
  → Mitigation: Use one lock per `Sound` and a separate manager-list lock; callback never holds list lock while entering decoder work.

## Migration Plan

1. Implement synchronized `Sound` decoder accessors and callback-facing mix method.
2. Update `SoundManager` callback and list mutation paths to use manager-level locking and snapshots.
3. Build Phoenix target and run focused playback/seek smoke validation.

Rollback is straightforward: revert the change set in `Sound` and `SoundManager`.

## Open Questions

- None at this time; implementation can proceed with existing section behavior.
