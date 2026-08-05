## Why

`Model::CalcInterpolatedPosition/Rotation/Scaling` dispara `assert(Factor >= 0.0f && Factor <= 1.0f)` con determinadas animaciones. El assert es un síntoma: la causa raíz está en `Model::FindPosition/FindRotation/FindScaling`, cuyo fallback (`assert(0); return 0;`) devuelve el **índice de keyframe equivocado** cuando `AnimationTime` cae fuera del rango de keys de ese canal.

Esto ocurre porque `AnimationTime` se normaliza contra el dominio **global** de la animación (`aiAnimation::mDuration`) mientras que `Find*` lo indexa contra el dominio **por canal** (`keys[0].mTime … keys[n-1].mTime`). Assimp no garantiza que ambos dominios coincidan: un canal puede empezar después de 0 o terminar antes de `mDuration`.

En Debug el usuario ve el assert. En Release los asserts desaparecen y el fallo se convierte en un **artefacto visual silencioso**: el hueso salta al primer segmento de su curva con un `Factor` enorme, produciendo poses explotadas o deformaciones bruscas.

## What Changes

- **Corregir el fallback de `Find*`**: cuando `AnimationTime` supera el último keyframe del canal, devolver el último segmento válido (`mNumXKeys - 2`) en lugar de `0`. Este es el arreglo del defecto real.
- **Sustituir el assert de `Factor` por un clamp a `[0, 1]`** en los tres `CalcInterpolated*`. Una vez el índice de segmento es correcto, el clamp deja de ser un parche y pasa a ser la semántica estándar de extrapolación *hold*: antes del primer key se mantiene la primera pose, después del último se mantiene la última.
- **Normalizar `AnimationTime` negativo** en `Model::boneTransform`: `fmod` conserva el signo del dividendo, y `AnimationTime` procede de una variable de expresión de script (`m_fAnimationTime`) que puede evaluar a negativo. Sumar `m_animDuration` al resultado negativo restaura el bucle correcto.
- **Guardas numéricas mínimas** para evitar que el clamp enmascare valores no finitos (`clamp` propaga NaN):
  - `m_animDuration <= 0` → no evaluar la jerarquía de huesos (evita `fmod(x, 0) == NaN`).
  - `DeltaTime <= 0` en los tres `CalcInterpolated*` → `Factor = 0` (evita división por cero con keys de tiempo duplicado).
- **Cerrar el underflow latente de `FindPosition`**: `CalcInterpolatedPosition` solo comprueba `mNumPositionKeys == 1`; con `0` keys, `mNumPositionKeys - 1` desborda el `unsigned int` y el bucle lee fuera de rango. `FindRotation` y `FindScaling` ya tienen `assert(mNumXKeys > 0)`; `FindPosition` no.
- **Acotar el log de diagnóstico**: `setAnimation` y `setCamera` se invocan una vez por frame desde las secciones, de modo que un índice inválido genera hoy un `Logger::error` por frame — y cada uno emite además un mensaje de red al editor. Se pasa a reportar cada valor inválido distinto una sola vez. Sin esto, la guarda de duración inválida introducida arriba tendría el mismo defecto.

Los cambios se limitan a [Model.cpp](../../../Engine/src/core/renderer/Model.cpp) (`boneTransform`, `setAnimation`, `setCamera`, `FindPosition`, `FindRotation`, `FindScaling`, `CalcInterpolatedPosition`, `CalcInterpolatedRotation`, `CalcInterpolatedScaling`) y a tres miembros privados nuevos en [Model.h](../../../Engine/src/core/renderer/Model.h) para el control del log. No hay cambios de API pública, ni de formato de datos, ni de shaders.

## Capabilities

### New Capabilities
- `model-animation-interpolation`: muestreo determinista y acotado de los canales de keyframes de Assimp (posición, rotación, escala) para el cálculo de transformaciones de huesos, incluyendo el comportamiento fuera del rango de keys de un canal y frente a entradas de tiempo degeneradas.

### Modified Capabilities
<!-- Ninguna. Las specs existentes (animated-model-load-performance, drawscene-glb-node-transform-fix) no describen requisitos de muestreo de keyframes. -->

## Impact

- **Código**: `Engine/src/core/renderer/Model.cpp` (9 funciones privadas y de configuración) y tres miembros privados nuevos en `Model.h`.
- **Comportamiento observable**: desaparecen los asserts en Debug; en Release desaparecen los saltos de pose en modelos cuyos canales no cubren toda la duración de la animación. Las animaciones que hoy funcionan correctamente no cambian: para `AnimationTime` dentro del rango de keys de un canal, el índice y el `Factor` calculados son idénticos a los actuales.
- **Consumidores**: todas las secciones que reproducen animaciones — `drawScene`, `drawSceneMatrix`, `drawSceneMatrixFolder`, `drawSceneMatrixInstanced`, `drawSceneMatrixInstancedFolder` — a través de `Model::Draw` y `ModelInstance::drawInstanced`.
- **Dependencias**: ninguna nueva. El clamp puede usar `std::clamp` (`<algorithm>`) o `glm::clamp`, ambos ya disponibles.
- **Riesgo**: bajo. Los cambios solo alteran rutas que hoy son estados de error (assert / índice incorrecto).
