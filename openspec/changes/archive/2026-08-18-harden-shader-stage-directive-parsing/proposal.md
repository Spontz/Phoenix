## Why

Compute shaders make normal GLSL preprocessor directives such as `#define` and `#ifdef` common in shader sources. Stage separation must recognize only Phoenix's `#type <stage>` directive, so preprocessor text and incidental `#type` text cannot be mistaken for a stage boundary.

## What Changes

- Define strict recognition rules for a shader-stage directive: `#type` must begin a source line and be followed by a supported stage name.
- Preserve GLSL preprocessor directives and non-directive text inside the current shader stage.
- Report malformed stage directives deterministically instead of attempting compilation with an invalid or empty stage.
- Validate compute shader sources containing `#define` and `#ifdef` with temporary regression coverage that is removed after execution.

## Capabilities

### New Capabilities
- `shader-stage-source-parsing`: Separates combined GLSL shader files into valid pipeline stages without consuming ordinary GLSL preprocessor directives.

### Modified Capabilities

- None.

## Impact

- Affected code: `Engine/src/core/renderer/Shader.cpp` and the focused parser test or smoke-test surface selected during implementation.
- No public shader syntax change: existing `#type vertex`, `#type fragment`, `#type geometry`, and `#type compute` declarations remain supported.
- No new runtime dependencies.