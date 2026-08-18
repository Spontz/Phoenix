# Shader Stage Source Parsing Specification

## Purpose

Define reliable separation of combined GLSL files into Phoenix shader stages while preserving standard GLSL preprocessor content.

## Requirements

### Requirement: Recognize explicit shader-stage directives

The shader loader SHALL treat a line as a stage boundary only when the line begins with the `#type` directive followed by a supported stage name: `vertex`, `fragment`, `geometry`, or `compute`. It SHALL preserve the established shader file syntax.

#### Scenario: Load a compute shader with GLSL conditionals

- **WHEN** a compute shader stage contains `#define`, `#ifdef`, `#ifndef`, or `#endif` lines after `#type compute`
- **THEN** the loader SHALL retain those lines in the compute-stage source and SHALL not create another shader stage from them

#### Scenario: Ignore a non-directive occurrence of the token

- **WHEN** source text contains `#type` outside the beginning of a stage-directive line
- **THEN** the loader SHALL leave that text in the current stage source rather than use it as a boundary

### Requirement: Reject malformed stage directives

The shader loader SHALL reject a stage-directive line that omits its stage name or names an unsupported stage, and SHALL report the shader-source error without compiling an invalid stage.

#### Scenario: Encounter an unsupported stage name

- **WHEN** a shader source contains a `#type` directive followed by an unsupported stage name
- **THEN** the shader load SHALL fail with an invalid-stage diagnostic

#### Scenario: Encounter a directive without a stage name

- **WHEN** a shader source contains a `#type` directive with no stage name
- **THEN** the shader load SHALL fail with a shader-source syntax diagnostic
