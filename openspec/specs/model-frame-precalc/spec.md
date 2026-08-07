# model-frame-precalc Specification

## Purpose
Define the per-frame contract by which Phoenix computes a model's animated state and then renders it, so that a section's script expressions can observe the model's animated camera and animated transforms before geometry is submitted, and so that the same animated state is computed once per frame regardless of how many times the model is drawn.
## Requirements
### Requirement: Two-phase per-frame model rendering

Phoenix SHALL render models through two distinct, separately invokable phases per frame: a **precalculation phase** that resolves the model's animated state without issuing any GPU work, and a **render phase** that submits GPU state and draw calls without recomputing that animated state.

The precalculation phase SHALL resolve, for a given playback time: the selected animation's node hierarchy, the bone transformation set, the animated transform of every mesh, and the view matrix of every camera defined inside the model file.

The render phase SHALL NOT sample animation keyframes, walk the node hierarchy, or resolve model cameras.

#### Scenario: Precalculation issues no GPU work

- **GIVEN** a loaded animated model
- **WHEN** the precalculation phase runs for a playback time
- **THEN** the model's animated state is available for reading
- **AND** no shader uniform is written and no draw call is issued

#### Scenario: Render phase consumes precalculated state

- **GIVEN** a model whose precalculation phase has run for the current frame
- **WHEN** the render phase runs
- **THEN** the geometry submitted reflects exactly the animated state produced by that precalculation
- **AND** the animation is not sampled again

#### Scenario: Rendering output is unchanged by the split

- **GIVEN** any section and model configuration that renders correctly before this change
- **WHEN** the same frame is rendered using the two-phase order
- **THEN** the resulting image is equivalent to the single-phase result
- **AND** shader uniforms carry the same values they carried before

### Requirement: Animated state is independent of the per-draw base transform

The animated transform a model computes for each of its meshes SHALL be independent of the base model transform supplied by the section for a particular draw, so that one precalculation result remains valid for every draw of that model in the frame.

The final transform used for a draw SHALL be the composition of the section-supplied base transform with the mesh's precalculated animated transform.

#### Scenario: Same model drawn at many base transforms

- **GIVEN** an animated model precalculated once for the current frame
- **WHEN** it is drawn repeatedly with a different base transform per draw
- **THEN** each draw is placed according to its own base transform
- **AND** every draw shows the same animation pose

#### Scenario: Repeated precalculation does not accumulate

- **GIVEN** a model whose precalculation phase is invoked more than once for the same playback time
- **WHEN** the model is rendered
- **THEN** the mesh transforms are identical to those produced by a single invocation
- **AND** no transform is applied twice

#### Scenario: Non-animated model

- **GIVEN** a model for which animation playback is disabled
- **WHEN** the precalculation and render phases run
- **THEN** each mesh is placed by the composition of the section base transform and its static scene-node transform
- **AND** the placement matches the behavior specified by the GLB node transform capability

### Requirement: Animated state is computed once per frame per model

Phoenix SHALL compute a model's animated state at most once per frame per distinct playback time, independently of the number of draws issued for that model in that frame.

#### Scenario: Instanced matrix section with many instances

- **GIVEN** a section that draws one animated model at N instance positions in a frame
- **WHEN** the frame is rendered
- **THEN** the node hierarchy is walked once for that frame, not once per instance
- **AND** the rendered result is identical to walking it per instance

#### Scenario: Folder section with several models

- **GIVEN** a section that draws several distinct models, each at several instance positions
- **WHEN** the frame is rendered
- **THEN** each distinct model has its animated state computed once for that frame

### Requirement: Model camera state is observable after precalculation

A model file MAY define one or more cameras, whose placement MAY be driven by the model's animation. After the precalculation phase, Phoenix SHALL expose the currently selected model camera's state — at minimum its view matrix, position, viewing direction, up vector, orientation angles and field of view — as readable values for the requested playback time.

Camera state exposed for a camera defined by a raw view matrix SHALL be derived from that matrix rather than left at default values.

#### Scenario: Animated camera inside a model file

- **GIVEN** a model containing an animated camera, with that camera selected
- **WHEN** the precalculation phase runs for a playback time
- **THEN** the exposed camera position and viewing direction correspond to the camera's animated placement at that time

#### Scenario: Camera selection out of range

- **GIVEN** a section that selects a camera index the model does not contain
- **WHEN** the precalculation phase runs
- **THEN** no model camera is used and the section's own view is preserved
- **AND** the condition is reported without repeating the report on every frame

#### Scenario: Model without cameras

- **GIVEN** a model that defines no cameras
- **WHEN** the precalculation phase runs
- **THEN** the model reports that no camera is available
- **AND** rendering falls back to the view supplied by the section

### Requirement: Script expressions can consume the model camera

Section script expressions SHALL be able to read the currently selected model camera's state for the frame being rendered, and values derived from it SHALL be usable to drive the model's placement and the section's shader variables in that same frame.

#### Scenario: Expression driven by the model camera

- **GIVEN** a section whose expression references the model camera position
- **WHEN** the section renders a frame
- **THEN** the expression evaluates against the camera state produced by that frame's precalculation
- **AND** the values it produces are applied to the geometry drawn in that same frame

#### Scenario: Playback time still comes from the expression

- **GIVEN** a section whose expression computes the animation playback time
- **WHEN** the section renders a frame
- **THEN** the playback time used for precalculation is the one the expression produced for that frame
- **AND** the model camera values exposed afterwards correspond to that playback time

#### Scenario: Expression that does not reference the model camera

- **GIVEN** a section whose expression references no model camera value
- **WHEN** the section renders a frame
- **THEN** the evaluated values are identical to those produced before this change

### Requirement: Motion blur history remains correct

Phoenix SHALL preserve the previous-frame transform history used by motion blur, advancing it exactly once per rendered draw and never as a side effect of the precalculation phase.

#### Scenario: Precalculation does not advance history

- **GIVEN** a model rendered with motion blur enabled
- **WHEN** the precalculation phase runs
- **THEN** the stored previous-frame transforms are unchanged

#### Scenario: History advances per draw

- **GIVEN** a model drawn at several instance positions in a frame
- **WHEN** the frame is rendered
- **THEN** each instance's previous transform corresponds to that same instance's transform in the preceding frame

### Requirement: Sections must precalculate before rendering a model

Every section that renders a model SHALL invoke the precalculation phase for that model before invoking its render phase in the same frame, including during shader warm-up execution.

#### Scenario: Warm-up execution

- **GIVEN** a section being warmed up for shader pre-caching
- **WHEN** the warm-up runs
- **THEN** the precalculation phase runs before the render phase
- **AND** the warm-up produces no error

#### Scenario: Render without precalculation

- **GIVEN** an animated model whose precalculation phase has never run
- **WHEN** its render phase is invoked
- **THEN** the model renders in a defined pose
- **AND** the engine does not crash or submit non-finite transforms

