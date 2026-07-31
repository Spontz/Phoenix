# sound-beat-detection Specification

## Purpose
TBD - created by archiving change fix-sound-beat-detection. Update Purpose after archive.
## Requirements
### Requirement: Beat remains a normalized envelope
Phoenix SHALL expose `beat` as a finite floating-point envelope in the inclusive range `[0.0, 1.0]`.

#### Scenario: No rhythmic attack
- **WHEN** the analyzed audio does not contain a detected rhythmic attack
- **THEN** `beat` approaches `0.0` according to the configured fade rate

#### Scenario: Rhythmic attack
- **WHEN** the detector recognizes a new rhythmic attack
- **THEN** `beat` reaches `1.0`
- **AND** subsequent envelope values remain between `0.0` and `1.0`

#### Scenario: Invalid or silent input
- **WHEN** the analysis history is uninitialized, silent, or otherwise unsuitable for a valid magnitude ratio
- **THEN** `beat` remains finite
- **AND** no startup beat is emitted solely because the baseline is zero

### Requirement: Beat represents low-frequency rhythmic attacks
Phoenix SHALL derive beat attacks from changes in low-frequency band magnitude rather than undifferentiated full-spectrum magnitude.

#### Scenario: Low-frequency transient
- **WHEN** low-frequency band magnitude rises sufficiently above its adaptive baseline
- **THEN** the detector emits a beat attack

#### Scenario: Isolated high-frequency tone
- **WHEN** a representative isolated tone remains above the configured beat band without material low-band magnitude
- **THEN** that tone does not by itself emit a beat attack

### Requirement: Detection timing is time-based
Phoenix SHALL calculate its adaptive baseline and envelope decay from elapsed time rather than a fixed number of rendered frames.

#### Scenario: Envelope at different frame rates
- **WHEN** equivalent detected attacks are updated at different stable render frame rates
- **THEN** envelope duration remains materially equivalent

#### Scenario: Frame-time spike
- **WHEN** one analysis update receives an unusually large frame time
- **THEN** the envelope remains within `[0.0, 1.0]`
- **AND** baseline adaptation remains finite

#### Scenario: First valid analysis window
- **WHEN** the detector receives its first valid non-silent analysis window
- **THEN** it initializes the adaptive baseline from that window
- **AND** startup suppression lasts no longer than that first valid window

### Requirement: Sustained magnitude produces one attack
Phoenix SHALL require the beat detector to rearm before another attack can be emitted.

#### Scenario: Magnitude stays above threshold
- **WHEN** low-frequency band magnitude crosses the attack threshold and then remains above it
- **THEN** one attack is emitted for that crossing
- **AND** the detector does not emit a new attack on every following frame

#### Scenario: Magnitude falls and rises again
- **WHEN** band magnitude falls below the rearm threshold after an attack and later crosses the attack threshold again
- **THEN** a new attack is emitted

### Requirement: Audio analysis uses a coherent sample snapshot
Phoenix SHALL prevent concurrent mutation of the FFT input while the main thread analyzes it.

#### Scenario: Audio callback overlaps analysis
- **WHEN** the audio callback produces samples while the main thread starts FFT analysis
- **THEN** the FFT operates on a coherent sample snapshot
- **AND** the audio callback does not block waiting to publish analysis samples

### Requirement: Current beat is available to sections
Phoenix SHALL update audio analysis before executing sections that consume the `beat` expression variable.

#### Scenario: Attack arrives before section execution
- **WHEN** an audio attack is available at the beginning of a rendered frame
- **THEN** sections executed in that frame observe the updated beat envelope

### Requirement: Existing beat consumers remain compatible
Phoenix SHALL retain the existing `beat` expression variable and adjustable ratio and fade controls.

#### Scenario: Existing expression
- **WHEN** an existing section evaluates an expression using `beat`
- **THEN** the expression continues to compile
- **AND** it receives a normalized envelope compatible with existing arithmetic uses

#### Scenario: Ratio and fade tuning
- **WHEN** the existing beat ratio or fade value is changed
- **THEN** the ratio continues to control attack sensitivity
- **AND** the fade continues to control envelope decay

### Requirement: Frequency-band values are normalized
Phoenix SHALL expose `low_freq_level`, `mid_freq_level`, and `high_freq_level` as finite floating-point levels in the inclusive range `[0.0, 1.0]`.

#### Scenario: Silent band
- **WHEN** a frequency band has no material signal above the configured dBFS floor
- **THEN** its exposed level is `0.0`

#### Scenario: Full-scale band
- **WHEN** a frequency band's RMS level is at or above `0 dBFS`
- **THEN** its exposed level is `1.0`

#### Scenario: Full-scale sine
- **WHEN** a coherent sine wave has sample peak amplitude `1.0`
- **THEN** its RMS level is approximately `-3.01 dBFS`
- **AND** its normalized band value is approximately `0.95`

#### Scenario: Intermediate band level
- **WHEN** a frequency band's RMS level is between `-60 dBFS` and `0 dBFS`
- **THEN** its exposed value is mapped logarithmically between `0.0` and `1.0`

#### Scenario: Invalid band input
- **WHEN** a calculated band level is non-finite or outside the supported range
- **THEN** the exposed value remains finite and clamped to `[0.0, 1.0]`

### Requirement: Frequency bands represent spectral RMS level
Phoenix SHALL derive each normalized band value from one-sided FFT power rather than a raw sum of FFT magnitudes.

#### Scenario: Frequency belongs to one band
- **WHEN** a non-DC FFT bin is analyzed
- **THEN** its power contributes to exactly one of the low, mid, or high bands according to the existing frequency boundaries

#### Scenario: Nyquist component
- **WHEN** the real FFT contains a Nyquist-bin component
- **THEN** its single-sided power contribution is included in the high-frequency band without doubling

#### Scenario: DC component
- **WHEN** the FFT contains a DC component
- **THEN** that component does not contribute to any exposed frequency-band level

#### Scenario: Different band widths
- **WHEN** bands contain different numbers of FFT bins
- **THEN** their values are calculated as signal RMS levels
- **AND** they are not raw magnitude sums whose scale grows linearly with bin count

### Requirement: Frequency-band level names describe their values
Phoenix SHALL expose frequency-band values through the `low_freq_level`, `mid_freq_level`, and `high_freq_level` expression names and retain the existing frequency boundaries.

#### Scenario: Frequency level expression
- **WHEN** a section evaluates one of the frequency-band level variables
- **THEN** the expression continues to compile
- **AND** it receives the normalized `[0.0, 1.0]` value for that band

#### Scenario: Frequency boundary
- **WHEN** audio is analyzed around the existing low/mid or mid/high boundary
- **THEN** bins are assigned using the current `m_lowFreqMax` and `m_midFreqMax` limits

