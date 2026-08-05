## Purpose

Define how Phoenix samples a skeletal animation's keyframe channels (position, rotation, scale) at an arbitrary playback time, so that bone transforms are always well-defined — including when the requested time falls outside the range covered by an individual channel's keyframes, or when the animation data or the requested time is degenerate.

## ADDED Requirements

### Requirement: Bounded interpolation factor

Phoenix SHALL produce an interpolation factor within the closed range `[0, 1]` for every keyframe channel sample, for any finite playback time and any animation accepted by the model loader.

The engine SHALL NOT abort, assert, or emit a non-finite bone transform because the requested playback time falls outside a channel's keyframe range.

#### Scenario: Playback time inside the channel range

- **GIVEN** an animation channel whose keyframes bracket the requested playback time
- **WHEN** the channel is sampled
- **THEN** the resulting pose is the linear (or spherical, for rotation) interpolation between the two bracketing keyframes
- **AND** the result is unchanged from the behavior prior to this change

#### Scenario: Playback time outside the channel range

- **GIVEN** an animation channel whose keyframes do not cover the requested playback time
- **WHEN** the channel is sampled
- **THEN** the interpolation factor is clamped into `[0, 1]`
- **AND** no assertion is raised

### Requirement: Hold extrapolation outside a channel's keyframe range

Because an animation's total duration is the maximum across all of its channels, an individual channel MAY begin after the animation start and MAY end before the animation end. For playback times outside a channel's own keyframe range, Phoenix SHALL apply *hold* extrapolation: the nearest boundary keyframe value is used unchanged.

#### Scenario: Channel that starts after the animation start

- **GIVEN** a channel whose first keyframe timestamp is greater than the animation start
- **WHEN** the animation is sampled at a time before that first keyframe
- **THEN** the channel yields exactly its first keyframe value
- **AND** the bone does not snap to an unrelated pose

#### Scenario: Channel that ends before the animation end

- **GIVEN** a channel whose last keyframe timestamp is less than the animation duration
- **WHEN** the animation is sampled at a time after that last keyframe
- **THEN** the channel yields exactly its last keyframe value
- **AND** the channel does NOT fall back to the segment between its first two keyframes

#### Scenario: Playback time exactly equal to the last keyframe timestamp

- **GIVEN** a channel sampled at a time numerically equal to its last keyframe timestamp, including when floating-point rounding of the wrapped playback time produces that equality
- **WHEN** the channel is sampled
- **THEN** the channel yields exactly its last keyframe value

### Requirement: Playback time normalization

Phoenix SHALL wrap the requested playback time into the animation's duration and SHALL present a non-negative time to the channel sampling stage, so that looping behaves identically whether the demo is played forwards, scrubbed, or evaluated at a negative time.

Playback time is supplied by section script expressions and is therefore NOT guaranteed to be monotonic or non-negative.

#### Scenario: Negative playback time

- **GIVEN** a section expression that evaluates the animation time to a negative value
- **WHEN** the animation is sampled
- **THEN** the time is wrapped into `[0, duration)` as if looping backwards
- **AND** the resulting pose matches the pose at the equivalent positive time

#### Scenario: Playback time beyond one animation loop

- **GIVEN** a playback time greater than the animation duration
- **WHEN** the animation is sampled
- **THEN** the time is wrapped into `[0, duration)` and the animation loops

### Requirement: Robustness against degenerate animation data

Phoenix SHALL sample animations without undefined behavior, division by zero, or non-finite output when animation data is degenerate, and SHALL report the condition rather than fail silently or crash.

#### Scenario: Animation with zero or unknown duration

- **GIVEN** a model whose selected animation has a duration of zero, or for which no valid animation could be selected
- **WHEN** a section requests animated rendering for that model
- **THEN** no bone transform is computed from an undefined time value
- **AND** the model renders in a defined pose instead of producing non-finite transforms
- **AND** the condition is reported through the engine log

#### Scenario: Channel with duplicated keyframe timestamps

- **GIVEN** two consecutive keyframes in a channel that share the same timestamp
- **WHEN** the channel is sampled across that segment
- **THEN** the interpolation factor is defined and finite
- **AND** the resulting pose is one of the two keyframe values, never a non-finite value

#### Scenario: Channel with no keyframes

- **GIVEN** an animation channel that contains zero keyframes of a given kind
- **WHEN** the channel is sampled
- **THEN** no out-of-bounds keyframe access occurs
- **AND** the corresponding component of the node transform is left at its default value

### Requirement: Bounded diagnostic reporting

Animation selection and playback state are re-applied by sections on every rendered frame. Phoenix SHALL report each distinct invalid animation or camera selection, and each animation with an invalid duration, without repeating the same report on every frame.

#### Scenario: Persistently invalid animation selection

- **GIVEN** a section that requests an animation index the model does not contain
- **WHEN** the section renders continuously for many frames
- **THEN** the condition is reported to the engine log
- **AND** the report is not repeated once per frame for the same index

#### Scenario: A different invalid selection appears later

- **GIVEN** an invalid animation or camera selection that has already been reported
- **WHEN** the section subsequently requests a *different* invalid index
- **THEN** the new index is reported as well

### Requirement: No regression for currently working animations

The sampling behavior for animations that already play correctly SHALL be unchanged.

#### Scenario: Animation whose channels span the full duration

- **GIVEN** an animation in which every channel's keyframes start at the animation start and end at the animation end
- **WHEN** the model is rendered across the full animation loop
- **THEN** every sampled pose is identical to the pose produced before this change
