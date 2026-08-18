## Context

`Shader.cpp` currently locates `#type` with an unrestricted substring search and then logs malformed input while continuing to build the stage map. This parser runs before shader compilation, but it lives beside OpenGL program management, while the repository has no established unit-test target. See [proposal.md](../proposal.md) and the capability spec for the required behavior.

## Goals / Non-Goals

**Goals:**
- Identify stage headers by complete source lines, not substring matches.
- Retain standard GLSL preprocessor directives and non-directive text exactly within their selected stage.
- Make malformed stage-header input a parse failure that prevents shader compilation.
- Exercise parsing without requiring an OpenGL context.

**Non-Goals:**
- Change the `#type <stage>` file format or add shader stages beyond the four supported stages.
- Add general GLSL preprocessing, include expansion, or macro evaluation.
- Change compute dispatch, image binding, or runtime 3D texture behavior.

## Decisions

### Scan source by lines and validate a complete directive

Use a line-oriented parser that recognizes `#type` only at the start of an active source line, followed by whitespace and exactly one supported stage name. The parser will track block-comment state so a token in a comment is never a directive. It will retain every non-header line in the active stage verbatim, including `#define`, `#ifdef`, and inline occurrences of `#type`.

This is preferred over another `find("#type")` predicate because line context and the directive grammar are necessary to distinguish Phoenix metadata from GLSL source. A complete GLSL lexer is unnecessary because only line-start directives and block comments affect this grammar.

### Represent parse failure explicitly

Return parse success or failure separately from the stage map, with a diagnostic that includes the offending directive line. `Shader::load` will stop before program compilation on failure.

This is preferred over logging and continuing because continuing can insert an invalid enum key or empty source and reports an unrelated compile/link error later.

### Validate the parser independently without retaining test infrastructure

Create a temporary OpenGL-free parser test using the project toolchain. Cases will cover compute `#define`/`#ifdef` content, inline or commented `#type` text, supported multi-stage sources, and malformed headers. Remove the temporary source and CMake wiring after the validation passes.

This is preferred over a renderer smoke test because the regression is deterministic text parsing and does not need GPU availability. The temporary target links only the code and dependencies required for validation and leaves no permanent test infrastructure.

## Risks / Trade-offs

- [Changing parser boundaries can reject shaders that relied on accidental substring recognition] -> Preserve all documented `#type <stage>` declarations and add representative existing shader resources to validation.
- [Block-comment tracking is not a full GLSL lexer] -> Restrict it to determining whether a line-start token is commented; do not reinterpret other source text.
- [Temporary test wiring can leave build configuration behind] -> Remove the test source and its CMake references after the validation succeeds.

## Migration Plan

1. Add the parser behavior and regression tests.
2. Build the new test target and run CTest.
3. Build the existing Engine target and smoke-load a compute shader containing GLSL preprocessor directives.
4. Roll back by reverting the parser/helper and test-target changes; shader source files retain their existing format.