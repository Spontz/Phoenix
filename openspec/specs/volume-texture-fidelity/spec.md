# Volume Texture Fidelity Specification

## Purpose

Upload image-based 3D textures as a faithful representation of their source files, so volume rendering can classify samples reliably, and report the loaded volume's value range so density thresholds can be chosen from real data.

## Requirements

### Requirement: Preserve source bit depth of image-based 3D textures

The engine SHALL determine the bit depth of the image files that compose a 3D texture and SHALL upload the volume without reducing that precision. A 16-bit source SHALL be stored in a 16-bit normalized internal format matching its channel count, and an 8-bit source SHALL keep its current 8-bit normalized internal format. Precision SHALL follow the source files and SHALL NOT be configurable.

#### Scenario: Load a 16-bit grayscale image sequence

- **WHEN** a 3D texture is loaded from single-channel 16-bit image files
- **THEN** the volume SHALL be uploaded with 16-bit precision per component, and a sample whose source value is below 1/256 of the full range SHALL remain distinguishable from zero when sampled by a shader

#### Scenario: Load an 8-bit image sequence

- **WHEN** a 3D texture is loaded from 8-bit image files
- **THEN** the volume SHALL be uploaded with the same internal format, channel count, and memory footprint as an 8-bit source before this change

#### Scenario: Slices disagree on bit depth

- **WHEN** the image files of one 3D texture do not all share the same bit depth
- **THEN** the load SHALL fail with a diagnostic naming the offending file, and no partially populated texture SHALL be exposed

### Requirement: Sample 3D textures at the uploaded level

The engine SHALL allocate exactly one level for a 3D texture and SHALL NOT generate a mipmap chain for it. Minification SHALL use the same filter as magnification, so a shader always reads the data as uploaded regardless of how it derives its sample coordinates.

#### Scenario: Ray-march a volume with unrelated screen-space derivatives

- **WHEN** a shader samples a 3D texture at coordinates whose screen-space derivatives do not correspond to the sampling density along the ray
- **THEN** the returned value SHALL come from the uploaded data and SHALL NOT be a reduced-resolution approximation of it

#### Scenario: Account for a loaded volume

- **WHEN** a 3D texture finishes loading
- **THEN** its reported memory SHALL be that of a single level, with no allowance for a mipmap chain

### Requirement: Report the value range of a loaded volume

The engine SHALL report, when a 3D texture is loaded from image files, the minimum and maximum sample values present in the volume normalized to the [0, 1] range that a shader observes, together with the selected internal format and the source bit depth.

#### Scenario: Load a volume whose data occupies part of the range

- **WHEN** a 3D texture is loaded whose highest sample is a small fraction of the format's full range
- **THEN** the log SHALL state the normalized minimum and maximum sample values, so a density threshold can be selected without inspecting the source files
