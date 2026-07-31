## 1. Safe FFT Input

- [x] 1.1 Add the minimal synchronization state needed to copy a coherent 2,048-sample FFT window without blocking the audio callback.
- [x] 1.2 Run Kiss FFT on the copied snapshot after releasing the audio callback lock.

## 2. Stable Beat Detector

- [x] 2.1 Remove the frame-count-based magnitude history and replace it with a finite time-based adaptive baseline initialized from the first valid window.
- [x] 2.2 Calculate beat level from non-DC low-frequency FFT magnitudes only.
- [x] 2.3 Add attack/rearm hysteresis so sustained above-threshold band magnitude emits one attack.
- [x] 2.4 Preserve `m_fBeatRatio` sensitivity and `m_fFadeOut` envelope decay while clamping `m_fBeat` to `[0.0, 1.0]`.
- [x] 2.5 Keep new signal-processing comments concise and in English.

## 3. Frame Integration

- [x] 3.1 Move FFT and beat analysis before section execution without changing behavior when sound or playback is disabled.
- [x] 3.2 Keep debug spectrogram updates based on the latest successful FFT analysis.

## 4. Normalized Frequency Bands

- [x] 4.1 Replace raw low, mid, and high magnitude sums with Parseval-normalized one-sided FFT RMS calculations that exclude DC and include Nyquist correctly.
- [x] 4.2 Map each band RMS from `[-60 dBFS, 0 dBFS]` to `[0.0, 1.0]` and clamp invalid or out-of-range values.
- [x] 4.3 Preserve the existing band boundaries and expose the normalized values through the new C++ and expression level names.
- [x] 4.4 Keep the band normalization implementation local and readable with English comments only where needed.

## 5. Validation

- [x] 5.1 Verify silence and startup never produce NaN, infinity, or a false beat caused by a zero baseline.
- [x] 5.2 Verify separated low-frequency pulses produce distinct `1.0` attacks with decay between them.
- [x] 5.3 Verify sustained low-frequency magnitude does not retrigger every frame and can retrigger after rearming.
- [x] 5.4 Verify a representative isolated high-frequency tone without material low-band magnitude does not trigger the detector.
- [x] 5.5 Compare envelope duration at multiple stable render frame rates and across a simulated frame-time spike.
- [x] 5.6 Verify silence maps all bands to `0.0` and coherent bin-centered tones well inside each band make the expected band dominant within a defined tolerance.
- [x] 5.7 Verify a coherent full-scale sine maps to approximately `0.95`, while intermediate and over-range signals follow the fixed dBFS mapping and remain finite in `[0.0, 1.0]`.
- [x] 5.8 Validate representative existing beat/frequency expressions, debug display, build diagnostics, and strict OpenSpec validation.
