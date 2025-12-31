#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 vWorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
	// Transform to world space.
    vec4 worldPos = model * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;

	gl_Position = projection * view * worldPos; 
}

#type fragment
#version 330 core

in vec2 TexCoords;
in vec3 vWorldPos;
out vec4 FragColor;

// Uniforms
uniform mat4 invModel;
uniform vec3 uCameraPos;         // The camera's position in world space (aka. eye pos)

uniform vec3 u_colorTint;
uniform float time;

// Volumen EN ESPACIO OBJETO
vec3 u_objMin = vec3(-0.5);
vec3 u_objMax = vec3( 0.5);

// Raymarching parameters.
float u_stepSize = 0.01;  // e.g. 0.01
int   u_numSteps = 200;  // e.g. 100


//-----------------------------------------------------------
//   TILEABLE HASH 3D  (REPETIBLE CADA “period” UNIDADES) 
//-----------------------------------------------------------

float tileHash(vec3 p, float seed, float period) {
    // Asegurar que las coordenadas de la celda sean periódicas:
    p = mod(p, period);

    // Hash periódico usando una mezcla trigonométrica + seed
    float h = dot(p, vec3(127.1 + seed, 311.7 + seed, 74.7 + seed));
    return fract(sin(h) * 43758.5453);
}

// Dirección de gradiente pseudoaleatoria
vec3 randomGradient(vec3 p, float seed, float period) {
    float r = tileHash(p, seed, period) * 6.28318530718; // ángulo
    float c = tileHash(p + 19.37, seed, period) * 2.0 - 1.0; // z -1..1
    float s = sqrt(1.0 - c * c);
    return vec3(cos(r) * s, sin(r) * s, c);
}

//-----------------------------------------------------------
//   PERLIN NOISE 3D TILEABLE
//-----------------------------------------------------------

float perlinTileable3D(vec3 p, float seed, float period) {
    // Hacer coordenadas periódicas
    p = mod(p, period);

    // Celdas base
    vec3 i0 = floor(p);
    vec3 f0 = fract(p);

    vec3 i1 = i0 + vec3(1.0);
    vec3 f1 = f0 - vec3(1.0);

    // Gradientes en las 8 esquinas, todas usando tileHash periódico
    vec3 g000 = randomGradient(i0, seed, period);
    vec3 g100 = randomGradient(vec3(i1.x, i0.y, i0.z), seed, period);
    vec3 g010 = randomGradient(vec3(i0.x, i1.y, i0.z), seed, period);
    vec3 g110 = randomGradient(vec3(i1.x, i1.y, i0.z), seed, period);

    vec3 g001 = randomGradient(vec3(i0.x, i0.y, i1.z), seed, period);
    vec3 g101 = randomGradient(vec3(i1.x, i0.y, i1.z), seed, period);
    vec3 g011 = randomGradient(vec3(i0.x, i1.y, i1.z), seed, period);
    vec3 g111 = randomGradient(i1, seed, period);

    // Productos punto
    float d000 = dot(g000, f0);
    float d100 = dot(g100, vec3(f1.x, f0.y, f0.z));
    float d010 = dot(g010, vec3(f0.x, f1.y, f0.z));
    float d110 = dot(g110, vec3(f1.x, f1.y, f0.z));

    float d001 = dot(g001, vec3(f0.x, f0.y, f1.z));
    float d101 = dot(g101, vec3(f1.x, f0.y, f1.z));
    float d011 = dot(g011, vec3(f0.x, f1.y, f1.z));
    float d111 = dot(g111, f1);

    // Fade clásico de Perlin
    vec3 u = f0 * f0 * (3.0 - 2.0 * f0);

    // Interpolación trilineal
    float noiseXY0 = mix(mix(d000, d100, u.x), mix(d010, d110, u.x), u.y);
    float noiseXY1 = mix(mix(d001, d101, u.x), mix(d011, d111, u.x), u.y);
    float n = mix(noiseXY0, noiseXY1, u.z);

    // Normalizar a 0..1
    return n * 0.5 + 0.5;
}

// Computes intersection of a ray (origin, direction) with an axis-aligned box defined by boxMin and boxMax.
vec2 intersectBox(vec3 orig, vec3 dir, vec3 boxMin, vec3 boxMax)
{
    vec3 invDir = 1.0 / dir;
    
    // Compute intersections with the slab boundaries.
    vec3 tMinTemp = (boxMin - orig) * invDir;
    vec3 tMaxTemp = (boxMax - orig) * invDir;
    
    vec3 tMin = min(tMinTemp, tMaxTemp);
    vec3 tMax = max(tMinTemp, tMaxTemp);
    
    float t0 = max(max(tMin.x, tMin.y), tMin.z);
    float t1 = min(min(tMax.x, tMax.y), tMax.z);
    
    return vec2(t0, t1);
}


void main() {
    // Compute the ray direction from the eye position to the fragment's world-space position.
    vec3 rayDir = normalize(vWorldPos - uCameraPos);
    
    // Convertir cámara al espacio local del cubo
    vec3 localCamPos = (invModel * vec4(uCameraPos, 1.0)).xyz;
    vec3 localRayDir = normalize((invModel * vec4(rayDir, 0.0)).xyz);

	// Compute where the ray enters and exits the world-space volume.
	vec2 tHit = intersectBox(localCamPos, localRayDir, u_objMin, u_objMax);
    
    // Discard the fragment if there is no valid intersection.
    if(tHit.x > tHit.y)
        discard;
    
    // Clamp start to 0 if the eye is inside the volume.
    float t0 = max(tHit.x, 0.0);
    float t1 = tHit.y;
  
	vec3 accumColor = vec3(0.0);
	float accumAlpha = 0.0;
	float t = t0;
	
	for (int i = 0; i < u_numSteps; i++) {
		if (t > t1)
			break;

		// Posición en el volumen
		vec3 pos = uCameraPos + t * rayDir;
		// Pasa pos (en world space) a local space del cubo
		vec3 localPos = localCamPos + t * localRayDir;
		
		vec3 texCoord = clamp(localPos + 0.5, 0.0, 1.0);

		// Densidad del voxel
		float seed = 1;
		float period = 5.0;                // repetición en unidades del espacio de ruido
		
		vec3 uvw = texCoord + vec3(time/4.0, 0, time/2.0);
		vec3 p = period * uvw;
		
		float density = perlinTileable3D(p, seed, period);

		// === Transfer function ===
		float alpha = density * u_stepSize;

		// === Color mapping ===
		vec3 color = vec3(density) * u_colorTint;
		
		// === Composición front-to-back (pre-multiplied alpha) ===
		accumColor += (1.0 - accumAlpha) * color * alpha;
		accumAlpha += (1.0 - accumAlpha) * alpha;

		// Early exit si ya es opaco
		if (accumAlpha >= 0.95)
			break;

		t += u_stepSize;
	}

	FragColor = vec4(accumColor, accumAlpha);
}