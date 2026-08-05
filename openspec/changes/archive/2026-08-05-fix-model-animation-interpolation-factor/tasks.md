## 1. Normalización del tiempo de reproducción

- [x] 1.1 En `Model::boneTransform` (Model.cpp:420), retornar sin evaluar la jerarquía cuando `m_animDuration <= 0`, registrando el error una vez con `Logger::error` (evita `fmod(x, 0) == NaN`).
- [x] 1.2 En `Model::boneTransform`, sumar `m_animDuration` a `AnimationTime` cuando el resultado de `fmod` sea negativo, para que el tiempo entregado a `ReadNodeHeirarchy` esté siempre en `[0, m_animDuration)`.

## 2. Corrección del índice de keyframe

- [x] 2.1 En `Model::FindPosition` (Model.cpp:509), sustituir el fallback `assert(0); return 0;` por `return pNodeAnim->mNumPositionKeys - 2;`.
- [x] 2.2 En `Model::FindRotation` (Model.cpp:521), sustituir el fallback `assert(0); return 0;` por `return pNodeAnim->mNumRotationKeys - 2;`.
- [x] 2.3 En `Model::FindScaling` (Model.cpp:534), sustituir el fallback `assert(0); return 0;` por `return pNodeAnim->mNumScalingKeys - 2;`.

## 3. Acotado del factor de interpolación

- [x] 3.1 En `Model::CalcInterpolatedPosition` (Model.cpp:547), sustituir `assert(Factor >= 0.0f && Factor <= 1.0f)` por un clamp de `Factor` a `[0, 1]`, y devolver `Factor = 0.0f` cuando `DeltaTime <= 0`.
- [x] 3.2 Aplicar el mismo cambio en `Model::CalcInterpolatedRotation` (Model.cpp:566).
- [x] 3.3 Aplicar el mismo cambio en `Model::CalcInterpolatedScaling` (Model.cpp:591).
- [x] 3.4 Verificar que se mantiene el `assert(NextXIndex < mNumXKeys)` en las tres funciones (invariante interno, no dato de entrada).

## 4. Canales sin keyframes

- [x] 4.1 En `Model::CalcInterpolatedPosition`, tratar `mNumPositionKeys == 0` devolviendo `aiVector3D(0, 0, 0)` antes de llamar a `FindPosition` (hoy el bucle desborda el `unsigned int`).
- [x] 4.2 En `Model::CalcInterpolatedRotation`, tratar `mNumRotationKeys == 0` devolviendo la identidad `aiQuaternion(1, 0, 0, 0)`.
- [x] 4.3 En `Model::CalcInterpolatedScaling`, tratar `mNumScalingKeys == 0` devolviendo `aiVector3D(1, 1, 1)`.

## 5. Acotado del log de diagnóstico

- [x] 5.1 Añadir a `Model.h` tres miembros `std::optional<unsigned int>` (`m_lastInvalidAnimation`, `m_lastInvalidCamera`, `m_lastInvalidAnimDuration`) y el `#include <optional>`.
- [x] 5.2 En `Model::setAnimation`, reportar el índice inválido solo cuando difiera del último ya reportado (hoy loguea por frame, con envío de red al editor incluido).
- [x] 5.3 Aplicar el mismo control en `Model::setCamera`, que tiene el mismo defecto y se invoca igualmente por frame.
- [x] 5.4 Usar el mismo patrón en la guarda de duración inválida de `Model::boneTransform` en lugar de un `bool` de un solo uso.

## 6. Validación

- [x] 6.1 Compilar el Engine en Debug y en Release sin nuevos warnings.
- [x] 6.2 Reproducir el modelo que originalmente disparaba el assert y confirmar que ya no salta en Debug.
- [x] 6.3 Comprobar visualmente que en ese modelo los huesos afectados mantienen la primera/última pose del canal en lugar de saltar al inicio de la curva.
- [x] 6.4 Verificar no-regresión en un modelo animado cuyos canales cubren toda la duración: la reproducción debe ser indistinguible de la anterior al cambio.
- [x] 6.5 Forzar un tiempo de animación negativo desde la expresión de la sección y confirmar que la animación hace bucle en lugar de congelarse o disparar el assert.
- [x] 6.6 Forzar un número de animación inválido con `playAnimation` activo y confirmar que se registra el error una sola vez y no se producen transformaciones no finitas.
- [x] 6.7 Ejecutar `openspec validate --strict fix-model-animation-interpolation-factor`.
