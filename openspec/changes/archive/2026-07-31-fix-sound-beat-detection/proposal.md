## Why

Phoenix exposes `beat` to section expressions as a normalized audio-reactive value, but the current detector compares full-spectrum magnitude against an unstable frame-count-dependent history. This can produce startup false positives, react to non-rhythmic high-frequency changes, and keep event-style effects triggered across multiple frames.

The exposed low, mid, and high frequency values are currently raw FFT magnitude sums. They are unbounded and biased by the very different number of bins in each band, so they cannot be used consistently as normalized audio-reactive inputs.

## What Changes

- Preserve `beat` as a continuous normalized envelope in the inclusive range `[0.0, 1.0]` for compatibility with existing expressions and shaders.
- Detect rhythmic attacks from low-frequency band magnitude using a stable time-based adaptive baseline.
- Prevent startup division errors and repeated attack triggering while the signal remains above threshold.
- Make FFT sample exchange between the audio callback and main thread safe.
- Make the latest beat analysis available before sections execute.
- Expose `low_freq_level`, `mid_freq_level`, and `high_freq_level` as finite normalized band levels in `[0.0, 1.0]` using a fixed dBFS scale.
- **BREAKING** Preserve the existing frequency boundaries while replacing the misleading `_sum` expression names with `_level`.
- Keep the implementation local and readable, without introducing BPM estimation, external dependencies, or a configurable analysis framework.
- Use English comments only where the signal-processing intent is not self-explanatory.

## Capabilities

### New Capabilities

- `sound-beat-detection`: Stable normalized rhythmic-attack and frequency-band analysis for audio-reactive engine expressions and graphics.

### Modified Capabilities

None.

## Impact

- Beat and FFT state in `Engine/src/core/sound/SoundManager.h`.
- Audio sample capture and beat analysis in `Engine/src/core/sound/SoundManager.cpp`.
- FFT execution order in `Engine/src/core/DemoKernel.cpp`.
- Existing expression variable `beat` remains available with the same `[0.0, 1.0]` contract.
