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

uniform float u_density;
uniform vec3 u_colorTint;
uniform float time;

// Volumen EN ESPACIO OBJETO
vec3 u_objMin = vec3(-0.5);
vec3 u_objMax = vec3( 0.5);

// Raymarching parameters.
float u_stepSize = 0.01;  // e.g. 0.01
int   u_numSteps = 100;  // e.g. 100


// ------------------------------------------------------------
// Intersección rayo / caja AABB
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


float sphereDensity(vec3 p, vec3 center, float radius)
{
    float d = length(p - center);
    return exp(-4.0 * (d*d) / (radius*radius)); // suave tipo gaussian
}

float renderVolume(vec3 p)
{
    float d = 0.0;

    // Esfera 1
    d += sphereDensity(p, vec3(0.5+0.2*sin(time), 0.5+0.2*cos(time), 0.5), 0.15);
    // Esfera 2
    d += sphereDensity(p, vec3(0.5+0.2*sin(time), 0.5, 0.5+0.2*cos(time)), 0.1);
    // Esfera 3
    d += sphereDensity(p, vec3(0.5, 0.5+0.2*sin(time*2), 0.5+0.2*cos(time)), 0.2);
	
    return d;  // densidad total en ese punto
}

void main() {

    // ========================================================
    // 1. Ray en espacio mundo
    vec3 rayDirWorld  = normalize(vWorldPos - uCameraPos);
    vec3 rayOrigWorld = uCameraPos;

    // ========================================================
    // 2. Transformar ray a ESPACIO OBJETO  (CLAVE)
    vec3 rayOrig = vec3(invModel * vec4(rayOrigWorld, 1.0));
    vec3 rayDir  = normalize(vec3(invModel * vec4(rayDirWorld, 0.0)));

    // ========================================================
    // 3. Intersección con la caja del volumen (objeto)
    vec2 tHit = intersectBox(rayOrig, rayDir, u_objMin, u_objMax);
    if(tHit.x > tHit.y)
        discard;

    float t0 = max(tHit.x, 0.0);
    float t1 = tHit.y;

    // ========================================================
    // 4. Raymarch en ESPACIO OBJETO
    vec3 accumColor = vec3(0.0);
    float accumAlpha = 0.0;
    float t = t0;

    for(int i = 0; i < u_numSteps; i++)
    {
        if(t > t1)
            break;

        vec3 posObj = rayOrig + t * rayDir;

        // Mapear de [-0.5,0.5] → [0,1]
        vec3 texCoord = posObj + vec3(0.5);

        float density = renderVolume(texCoord) * u_density;
        float alpha = density;

        accumColor += (1.0 - accumAlpha) * u_colorTint * alpha;
        accumAlpha += (1.0 - accumAlpha) * alpha;

        if(accumAlpha >= 0.95)
            break;

        t += u_stepSize;
    }

    FragColor = vec4(accumColor, accumAlpha);
	//FragColor = vec4(1,0,0,1);
}