## 1. Parser Implementation

- [x] 1.1 Extract or isolate shader-stage source parsing so it can return parsed stages or an explicit diagnostic without creating an OpenGL program.
- [x] 1.2 Replace unrestricted `#type` substring scanning with complete line-directive recognition, including block-comment awareness and supported-stage validation.
- [x] 1.3 Stop `Shader::load` before compilation when stage parsing fails, preserving the existing accepted `#type <stage>` syntax.

## 2. Regression Coverage

- [x] 2.1 Execute an OpenGL-free temporary parser test, then remove its source files and CMake references.
- [x] 2.2 Cover compute source containing `#define`, `#ifdef`, `#ifndef`, and `#endif`, verifying the directives remain in one compute stage.
- [x] 2.3 Cover supported multi-stage input, inline/commented `#type` text, missing stage names, and unsupported stage names.

## 3. Validation

- [x] 3.1 Configure and build the parser test target, then run its CTest case.
- [x] 3.2 Build the Engine target and smoke-load a compute shader resource with GLSL preprocessor directives.
- [x] 3.3 Run `openspec validate harden-shader-stage-directive-parsing --strict` and record any validation limitation.