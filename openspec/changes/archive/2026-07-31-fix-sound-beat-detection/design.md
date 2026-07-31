## Context

`SoundManager::performFFT()` currently sums all FFT magnitudes, divides the result by a history average, and emits an envelope when that ratio exceeds `m_fBeatRatio`. The envelope is exposed as `beat` through `MathDriver`.

The current implementation has several correctness issues:

- the initial average can be zero, producing an invalid ratio and a false startup attack;
- the history spans 1,024 render frames, so its real duration and response depend on frame rate;
- full-spectrum magnitude reacts to non-rhythmic high-frequency changes;
- an above-threshold signal resets intensity to `1.0` every frame instead of producing one attack;
- the audio callback can modify the FFT input while the main thread reads it;
- analysis runs after section execution, adding one frame of avoidable latency.

The frequency-band variables have a separate normalization problem. They directly sum FFT magnitudes across approximately 18 non-DC low, 74 mid, and 932 high bins. Their values are unbounded, and the high band has a much larger numerical scale simply because it covers more bins.

Existing shaders and expressions use `beat` as a normalized continuous modulation value, so changing it to BPM, an unbounded magnitude value, or a persistent boolean would be incompatible.

## Goals / Non-Goals

**Goals:**

- Keep `beat` finite and within `[0.0, 1.0]`.
- Detect useful low-frequency rhythmic attacks for visual synchronization.
- Make sensitivity and fade behavior time-based.
- Emit one attack per magnitude crossing.
- Remove the FFT input data race with a small synchronization boundary.
- Provide the current analysis before sections render.
- Expose low, mid, and high band levels on a fixed, documented `[0.0, 1.0]` scale.
- Keep existing band boundaries and use names that describe normalized levels.
- Keep changes concentrated in `SoundManager` and the main-loop call order.

**Non-Goals:**

- Estimate BPM, beat phase, bars, or musical meter.
- Guarantee semantic beat detection for every music genre.
- Add a third-party beat-tracking library.
- Add multiple beat channels or frequency-band configuration in this change.
- Add adaptive per-band peak normalization or automatic gain control.
- Redesign FFT visualization or the audio mixer.
- Replace public fields or existing expression names.

## Decisions

### 1. Preserve `beat` as an attack envelope

On a new attack, set `m_fBeat` to `1.0`. On subsequent updates, decay it toward zero using `m_fFadeOut` and elapsed time. Clamp the final value to `[0.0, 1.0]`.

This preserves current modulation uses while still allowing an event consumer to detect a rising edge or threshold crossing.

Alternative considered: expose `beat` as a one-frame boolean pulse. Rejected because existing shaders expect a smooth value.

Alternative considered: expose BPM. Rejected because BPM describes tempo and does not identify the current attack amplitude or timing by itself.

### 2. Detect attacks from low-frequency band magnitude

Calculate beat level from FFT magnitudes above DC and at or below the existing low-frequency boundary. Compare this band magnitude against an adaptive baseline.

This focuses detection on kick and bass transients and reuses the existing frequency mapping without adding configuration.

Alternative considered: keep full-spectrum magnitude. Rejected because vocals, cymbals, and broadband volume changes can trigger visual beats without a rhythmic low-frequency pulse.

### 3. Replace frame history with a time-based exponential baseline

Maintain one adaptive baseline value instead of a 1,024-element frame history. Update it using an exponential coefficient derived from `frameTime`. Seed it from the first valid analysis window and suppress detection for that window only.

This removes the buffer, the O(FFT_SIZE) shift, the zero-filled startup bias, and frame-rate-dependent history duration.

The baseline must only be updated from finite non-negative magnitude and must never be used as a divisor below a small positive epsilon.

Alternative considered: keep a smaller circular history buffer. Rejected because it remains frame-count based and requires more state than an exponential baseline.

### 4. Use hysteresis to rearm the detector

After an attack, mark the detector unarmed. Rearm only after band magnitude falls below a lower threshold relative to the adaptive baseline. The existing `m_fBeatRatio` remains the upper attack threshold.

Hysteresis prevents a sustained loud passage from generating one attack every frame without adding a timer or tempo model.

### 5. Snapshot samples without blocking the audio callback

Protect only copying between the callback-owned sample buffer and a main-thread snapshot. The main thread briefly locks while copying. The audio callback uses a non-blocking lock attempt and skips publishing that FFT window if the snapshot is being copied.

Release the lock before running Kiss FFT and beat calculations. The audio callback must never wait for the main thread.

Alternative considered: atomics for every sample. Rejected because they complicate the buffer and do not provide a coherent window.

Alternative considered: a lock-free double buffer. Rejected for this change because a non-blocking publication attempt is simpler and dropping an occasional analysis window does not affect audio output.

### 6. Analyze before executing sections

Move the existing `performFFT()` call to the frame stage immediately before section execution when sound is enabled and the demo is playing. Keep debug spectrogram filling after a successful analysis.

This removes one frame of engine-side latency without changing section APIs.

### 7. Keep comments sparse and in English

Only comment the adaptive-baseline and hysteresis intent where the code is not self-explanatory. Do not add a new abstraction layer for this small algorithm.

### 8. Normalize band RMS using a fixed dBFS range

For FFT length `N = 2048`, calculate each band's mean square directly from the unscaled `kiss_fftr` complex output:

- exclude DC (`k = 0`);
- for interior bins `1 <= k < N/2`, contribution is `2 * |X[k]|^2 / N^2`;
- for Nyquist (`k = N/2`), contribution is `|X[k]|^2 / N^2`;
- band RMS is the square root of the sum of its bin contributions.

Use sample peak amplitude `1.0` as the dBFS reference: `dBFS = 20 * log10(RMS)`. A full-scale sine therefore has RMS `1/sqrt(2)`, measures approximately `-3.01 dBFS`, and maps to approximately `0.95`.

Map the fixed range `[-60 dBFS, 0 dBFS]` linearly to `[0.0, 1.0]`, clamping values outside that range.

This gives stable semantics:

- `0.0` means silence or a band level at/below `-60 dBFS`;
- `1.0` means a band RMS level at/above `0 dBFS`;
- intermediate values represent logarithmic signal level.

Calculate power from the FFT length and complex output rather than from the existing raw magnitude sums. This avoids a linear bin-count bias while measuring the total signal level present in each band. Exclude DC and assign every remaining one-sided bin, including Nyquist, to exactly one existing band.

Rename the public members to `m_fLowFreqLevel`, `m_fMidFreqLevel`, and `m_fHighFreqLevel`. Expose them to expressions as `low_freq_level`, `mid_freq_level`, and `high_freq_level` so their names match the normalized level semantics.

Alternative considered: normalize each band against its recent maximum. Rejected because quiet background noise would eventually become `1.0`, and identical audio could produce different values depending on recent history.

Alternative considered: divide the magnitude sum by the number of bins. Rejected because that measures average spectral magnitude rather than the RMS signal level present in the band and does not provide a defined full-scale reference.

## Risks / Trade-offs

- **[Bass-light music produces fewer attacks]** -> Retain the adjustable ratio and existing low-frequency boundary for tuning.
- **[An FFT window is skipped during contention]** -> Keep the previous coherent window; audio playback always takes priority over analysis freshness.
- **[Moving analysis changes timing by one frame]** -> This is intentional; verify section execution and debug display remain ordered correctly.
- **[Existing content was tuned to broadband magnitude]** -> Preserve range, ratio, and fade controls; validate representative existing beat-driven scenes.
- **[Long frame stalls decay the envelope immediately]** -> Clamp time-dependent coefficients and final envelope values to finite valid ranges.
- **[Existing expressions used the old `_sum` names]** -> Document the intentional rename and validate representative scenes.
- **[Several mixed sounds exceed digital full scale]** -> Clamp each normalized band value to `1.0`.

## Migration Plan

1. Add coherent FFT sample snapshot state.
2. Replace frame-history magnitude averaging with an initialized adaptive baseline.
3. Limit attack magnitude to the low-frequency band and add hysteresis.
4. Preserve normalized envelope decay and existing controls.
5. Replace raw band sums with normalized one-sided FFT RMS levels mapped through the fixed dBFS range.
6. Move analysis before section execution.
7. Validate silence, synthetic tones in every band, mixed signals, pulses, sustained tones, high-frequency transients, and representative existing scenes.

Rollback consists of restoring the previous detector state and frame call order.

## Open Questions

- What low-frequency boundary and default ratio provide the best result for Phoenix's representative music set?
- Should a distinct one-frame event variable be proposed later if sections need event semantics without rising-edge logic?
