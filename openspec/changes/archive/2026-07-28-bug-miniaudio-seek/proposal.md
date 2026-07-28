## Why

`Sound::seekSound()` can run on the main thread while the miniaudio callback thread is reading from the same decoder. This unsynchronized access can cause random memory faults and unstable playback behavior.

## What Changes

- Add thread-safe synchronization for per-sound decoder access during seek, read, load/unload, and playback state transitions.
- Guard `SoundManager` sound-list access so callback iteration cannot race with add/clear operations.
- Keep existing playback behavior (seek by seconds, stop on EOF) while removing unsafe concurrent decoder access.
- Add focused verification coverage for repeated seek while playback callback is active.

## Capabilities

### New Capabilities
- `miniaudio-thread-safe-seek`: Safe concurrent sound seek and playback mixing without decoder data races.

### Modified Capabilities
- None.

## Impact

- `Engine/src/core/sound/Sound.h`
- `Engine/src/core/sound/Sound.cpp`
- `Engine/src/core/sound/SoundManager.h`
- `Engine/src/core/sound/SoundManager.cpp`
- Related runtime seek/playback flows that call `Sound::seekSound()`.
