## Context

`Model::boneTransform` normaliza el tiempo de reproducción contra el dominio **global** de la animación y luego `Model::Find{Position,Rotation,Scaling}` lo indexa contra el dominio **por canal**:

```
boneTransform(timeInSeconds)                            Model.cpp:420
   │  TicksPerSecond = anim->mTicksPerSecond (o 25 si vale 0)
   │  TimeInTicks    = timeInSeconds * TicksPerSecond
   │  AnimationTime  = fmod(TimeInTicks, m_animDuration)  ← m_animDuration = anim->mDuration
   ▼
ReadNodeHeirarchy(AnimationTime, ...)                   Model.cpp:435
   ▼
Find{Position,Rotation,Scaling}(AnimationTime, channel)  Model.cpp:509-545
   ▼
Factor = (AnimationTime - k[i].mTime) / (k[i+1].mTime - k[i].mTime)
   ▼
assert(Factor >= 0.0f && Factor <= 1.0f)                ← salta aquí
```

Assimp **no garantiza** que ambos dominios coincidan. `mDuration` es el máximo entre todos los canales, de modo que un canal concreto puede empezar después de `0` o terminar antes de `mDuration`:

```
   Dominio global normalizado por fmod:  [0, mDuration)
   ├──────────────────────────────────────────────────────┤
   0                                                  mDuration

   Canal A (arranca tarde y acaba pronto):
              ├───────────────────────────────────┤
             k0                                 kn-1
   ▲ zona A                                            ▲ zona B

   Zona A → Find* devuelve 0, Factor = (t - k0)/Δ  <  0   (sin assert(0))
   Zona B → el bucle se agota → assert(0) → return 0
            → Factor = (t - k0)/Δ  >>  1
```

### Causas raíz identificadas

| # | Causa | Factor resultante | Asserts que saltan |
|---|---|---|---|
| 1 | Canal cuyo primer key tiene `mTime > 0` (clips que no empiezan en el frame 0) | `< 0` | solo el de `Factor` |
| 2 | Canal cuyo último key acaba antes de `mDuration` | `>> 1` | `assert(0)` en `Find*` y luego el de `Factor` |
| 3 | `AnimationTime` negativo: `fmod` conserva el signo del dividendo, y el tiempo procede de una expresión de script (`m_fAnimationTime`) | `< 0` | solo el de `Factor` |
| 4 | Redondeo `double→float` en `(float)fmod(...)`: puede igualar el último key, y la comparación en `Find*` es `<` estricta | `>> 1` | ambos |
| 5 | `m_animDuration == 0` → `fmod(x, 0)` = `NaN` | `NaN` | ambos (toda comparación con NaN es falsa) |
| 6 | `DeltaTime == 0` (keys con tiempo duplicado) | `±inf` / `NaN` | ambos |

Las causas 1, 3 y 5 explican `Factor < 0`; las causas 2, 4 y 6 explican `Factor > 1`.

## Goals / Non-Goals

**Goals**
- Eliminar el assert manteniendo la pose **correcta**, no solo evitando el aborto.
- Cambio mínimo, contenido en `Model.cpp`, sin tocar API, formato de datos ni shaders.
- Preservar bit a bit el comportamiento actual para tiempos dentro del rango de keys de un canal.

**Non-Goals**
- No se reescribe el muestreo de animaciones ni se extrae una clase `Bones` (ya anotado como TODO en `Model.cpp:407`).
- No se cachea la búsqueda de keyframes ni se optimiza la búsqueda lineal.
- No se añaden modos de extrapolación configurables (loop / ping-pong). Solo *hold*.
- No se modifica cómo las secciones calculan `m_fAnimationTime`.

## Decisions

### Decisión 1: Corregir el índice devuelto por `Find*`, no solo acotar `Factor`

**Opciones consideradas**

| Opción | Efecto en la zona B (tiempo posterior al último key) | Veredicto |
|---|---|---|
| A. Solo `clamp(Factor, 0, 1)` | `Find*` sigue devolviendo `0`, así que `Factor = 1` interpola hasta `k[1]`: la pose **salta al principio de la curva** | Rechazada: cambia un assert por un pop visual silencioso |
| B. Solo corregir `Find*` | El índice pasa a ser el último segmento, pero `Factor` sigue saliendo `> 1` en el caso 4 y `< 0` en los casos 1 y 3 | Insuficiente |
| C. Corregir `Find*` **y** acotar `Factor` | Zona A → `Factor = 0` sobre el segmento 0 → primer key. Zona B → `Factor = 1` sobre el último segmento → último key | **Elegida** |

Con la opción C el clamp deja de ser un parche defensivo y pasa a ser la definición del comportamiento: extrapolación *hold*, que es la semántica que aplican los reproductores glTF y FBX y la propia utilidad de animación de Assimp.

Cambio concreto en las tres funciones `Find*`:

```
   fallback actual:   assert(0); return 0;
   fallback nuevo:    return pNodeAnim->mNumXKeys - 2;   // último segmento válido
```

Es seguro porque las tres funciones solo se alcanzan desde `CalcInterpolated*` después de descartar el caso `mNumXKeys == 1`, así que `mNumXKeys >= 2` y `mNumXKeys - 2` no desborda.

### Decisión 2: `clamp` en lugar de `assert`

El `assert` desaparece en Release, que es precisamente donde el defecto se vuelve invisible y produce el artefacto visual. Sustituirlo por `clamp` da el mismo comportamiento en Debug y en Release.

Se mantiene el `assert(NextXIndex < mNumXKeys)` existente: esa condición sí es un invariante interno que nunca debe romperse tras la Decisión 1, y su violación indicaría un error de programación, no un dato de entrada inusual.

### Decisión 3: Normalizar el `fmod` negativo en el origen

`AnimationTime` negativo se corrige donde se genera, en `boneTransform`, y no en cada `CalcInterpolated*`:

```
   AnimationTime = fmod(TimeInTicks, m_animDuration);
   if (AnimationTime < 0) AnimationTime += m_animDuration;   // fmod conserva el signo
```

Corregirlo aguas arriba mantiene un único punto de verdad y evita que cada canal aplique *hold* sobre su primer key cuando lo que el usuario espera es un bucle.

### Decisión 4: Guardas antes del clamp, no después

`std::clamp` con un `NaN` devuelve el propio `NaN`, y `glm::clamp` es igualmente indefinido. Por tanto el clamp **no** protege frente a las causas 5 y 6; hay que impedir que el `NaN` se genere:

- `m_animDuration <= 0` → `boneTransform` retorna sin evaluar la jerarquía y registra el error. Este caso ya es alcanzable hoy: `setAnimation` deja `m_animDuration` en `0` cuando el índice de animación es inválido pero `playAnimation` sigue activo.
- `DeltaTime <= 0` → `Factor = 0` (se toma el key de inicio del segmento).

### Decisión 5: Cerrar el underflow latente en el canal de posición

`CalcInterpolatedPosition` solo descarta `mNumPositionKeys == 1`; con `0` keys, el bucle de `FindPosition` evalúa `i < 0u - 1` y lee fuera de rango. `FindRotation` y `FindScaling` ya tienen `assert(mNumXKeys > 0)`, pero un assert tampoco protege en Release. Se unifican las tres rutas comprobando `mNumXKeys == 0` en los `CalcInterpolated*` y devolviendo el valor por defecto del componente (vector cero para posición, `(1,0,0,0)` para rotación, `(1,1,1)` para escala), que es la identidad de la transformación correspondiente.

### Decisión 6: Acotar el log de los setters que corren por frame

`setAnimation` y `setCamera` se invocan desde `exec()` de cada sección, es decir una vez por frame. Con un índice inválido, el `Logger::error` actual se dispara a 60 Hz, y cada llamada además emite un mensaje de red al editor vía `NetDriver::sendMessage`. La guarda de duración inválida de la Decisión 4 tendría exactamente el mismo problema.

Se recuerda el **último valor inválido reportado** con `std::optional<unsigned int>` (idioma ya presente en `EditorApiServer.cpp` y `FramebufferStreamer.cpp`) en lugar de un simple `bool`:

| Enfoque | Un índice inválido repetido | Un índice inválido *distinto* después |
|---|---|---|
| `bool` "ya avisado" | reportado una vez ✅ | **silenciado** ❌ |
| `std::optional` último valor | reportado una vez ✅ | reportado ✅ |

El `optional` vacío compara distinto de cualquier valor, así que el primer reporte funciona sin necesidad de un valor centinela. No se limpia al seleccionar un índice válido: el mensaje comunica un hecho estático del fichero ("la animación 5 no existe"), y repetirlo no aporta información.

## Risks / Trade-offs

- **La corrección puede cambiar el aspecto de alguna animación existente.** Es intencionado: los modelos afectados hoy renderizan una pose incorrecta en Release. Mitigación: validar visualmente los modelos animados del proyecto antes y después.
- **El clamp silencia datos anómalos.** Mitigación: los casos verdaderamente degenerados (duración `<= 0`) sí se registran en el log; el *hold* fuera de rango es comportamiento normal y esperado, no se loguea para no inundar el log por frame y por hueso.
- **Coste**: una comparación y una posible suma por llamada. Despreciable frente a la búsqueda lineal de keyframes que ya existe.

## Migration Plan

No aplica. El cambio es interno a `Model.cpp`, no altera ninguna firma pública ni ningún formato de datos, y no requiere reexportar ni migrar assets.

## Open Questions

- ¿Se quiere un log *una sola vez por modelo* cuando se detecta un canal que no cubre toda la duración de la animación? Ayudaría a diagnosticar exportaciones defectuosas, pero añade estado por modelo y queda fuera del alcance mínimo de este cambio.
