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
uniform sampler3D uVolumeTex;    // 3D texture holding your volume (density and temperature).
uniform float u_densityScale;
uniform vec3 u_colorTint;
uniform float u_time;

// Raymarching parameters.
float u_stepSize = 0.01;  // e.g. 0.01
int   u_numSteps = 200;  // e.g. 100


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
    
	// El cubo está en espacio local [-0.5, 0.5]
    vec3 localMin = vec3(-0.5);
    vec3 localMax = vec3(0.5);

    // Convertir cámara al espacio local del cubo
    vec3 localCamPos = (invModel * vec4(uCameraPos, 1.0)).xyz;
    vec3 localRayDir = normalize((invModel * vec4(rayDir, 0.0)).xyz);
	
	// Compute where the ray enters and exits the world-space volume.
	vec2 tHit = intersectBox(localCamPos, localRayDir, localMin, localMax);
    
    // Discard the fragment if there is no valid intersection.
    if(tHit.x > tHit.y)
        discard;
    
    // Clamp start to 0 if the eye is inside the volume.
    float t0 = max(tHit.x, 0.0);
    float t1 = tHit.y;
    
	vec3 accumColor = vec3(0.0);
	float accumAlpha = 0.0;
	float t = t0;
	
	// Debug: pinta el cubo en rojo sólido
	// FragColor = vec4(1,0,0,1);
	// return;

	for (int i = 0; i < u_numSteps; i++) {
		if (t > t1)
			break;

		// Posición en el volumen
		vec3 pos = uCameraPos + t * rayDir;
		// Pasa pos (en world space) a local space del cubo
		 vec3 localPos = localCamPos + t * localRayDir;
		
		vec3 texCoord = clamp(localPos + 0.5, 0.0, 1.0);

		// Densidad del voxel tal cual aparece en la textura
		float density =  u_densityScale * texture(uVolumeTex, texCoord).r;
			
		// Dibujamos la imagen progresivamente
		/*
		float density = 2.0;
		float zero_to_one = max(1-u_time/10.0, 0.0);
		if (texCoord.y > zero_to_one)
			density = u_densityScale * texture(uVolumeTex, texCoord).r;
		*/
		
		// === Transfer function ===
		// Ajusta los valores 0.2 y 0.6 a tu dataset CT
		float alpha = smoothstep(0.2, 0.6, density) * u_stepSize;
		//float alpha = density * u_stepSize;

		// === Color mapping ===
		// Escala de grises (negro=aire, blanco=hueso)
		vec3 color = vec3(0);
		if (density<2.0) {
			alpha = 0.0;
			color = vec3(density,0,0) * u_colorTint;
			}
		else
			color = vec3(density) * u_colorTint;
		
		// Let's do some color changes
		//-----------------------------------------
		// 1. Gradiente vertical según texCoord.y
		//-----------------------------------------
		float h = texCoord.y;            // 0 = abajo, 1 = arriba

		// Colores base del gradiente (puedes cambiarlos)
		vec3 bottomColor = vec3(0.2, 0.3, 0.8);   // azul
		vec3 topColor    = vec3(1.0, 0.5, 0.0);   // naranja

		vec3 heightColor = mix(bottomColor, topColor, h);

		//-----------------------------------------
		// 2. Animación temporal con una paleta continua
		//-----------------------------------------
		float time = u_time * 0.5;   // velocidad del cambio de color
		float wave = sin(6.2831 * (h + time));     // animación suave 0→1→0
		float wave2 = cos(6.2831 * (h*0.5 + time));

		// Paleta animada
		vec3 timeColor = vec3(
			0.5 + 0.5 * wave,
			0.5 + 0.5 * wave2,
			0.5 + 0.5 * sin(time + h * 5.0)
		);

		//-----------------------------------------
		// 3. Combinar el color por altura, animación y densidad
		//-----------------------------------------
		color *= mix(heightColor, timeColor, 0.5);  // puedes ajustar este 0.5
		
	

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