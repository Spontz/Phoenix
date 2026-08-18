#type vertex
#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 LocalPos;
out vec3 CameraLocalPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    LocalPos = aPos;

    /*
     * Camera position in local/object space.
     *
     * The camera is the origin in view space, so:
     *
     * worldCamera = inverse(view) * vec4(0,0,0,1)
     * localCamera = inverse(model) * worldCamera
     */
    vec3 cameraWorld = (inverse(view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    CameraLocalPos = (inverse(model) * vec4(cameraWorld, 1.0)).xyz;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}


#type fragment
#version 330 core

in vec3 LocalPos;
in vec2 TexCoords;
in vec3 CameraLocalPos;

out vec4 FragColor;

uniform sampler3D volume;


// Volume bounds in local space. Assumes the cube goes from -0.5 to +0.5.
const vec3 BOX_MIN = vec3(-0.5);
const vec3 BOX_MAX = vec3( 0.5);


// Number of samples.
const int NUM_STEPS = 128;

// Density multiplier.
uniform float density;

bool intersectBox(
    vec3 rayOrigin,
    vec3 rayDir,
    vec3 boxMin,
    vec3 boxMax,
    out float tEnter,
    out float tExit
)
{
    // Avoid division by zero.
    vec3 invDir = 1.0 / rayDir;
    vec3 t0 = (boxMin - rayOrigin) * invDir;
    vec3 t1 = (boxMax - rayOrigin) * invDir;

    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);

    tEnter = max( max(tMin.x, tMin.y), tMin.z);

    tExit = min( min(tMax.x, tMax.y), tMax.z);

    return tExit >= max(tEnter, 0.0);
}


void main()
{
    // The ray starts at the camera.
    vec3 rayOrigin = CameraLocalPos;

	// We use the current cube surface position to determine the ray direction.
    vec3 rayDir = normalize(LocalPos - rayOrigin);

    // Find intersection with the volume box./
    float tEnter;
    float tExit;

    if (!intersectBox(
		rayOrigin,
        rayDir,
        BOX_MIN,
        BOX_MAX,
        tEnter,
        tExit))
    {
        discard;
    }


    // If the camera is outside:
    //     tEnter > 0
    // If the camera is inside:
    //     tEnter < 0
    // Therefore start at max(tEnter, 0).
    float tStart = max(tEnter, 0.0);

    float distance = tExit - tStart;

    if (distance <= 0.0)
        discard;


    // Ray marching step.
    float step = distance / float(NUM_STEPS);
    // Start ray.
    vec3 samplePos = rayOrigin + rayDir * tStart;

    // Accumulated opacity.
    float accumulatedAlpha = 0.0;

    // Accumulated color.
    vec3 accumulatedColor = vec3(0.0);

    for (int i = 0; i < NUM_STEPS; ++i)
    {
        //Convert local cube coordinates [-0.5, +0.5], to texture coordinates [0, 1]
        vec3 uv = samplePos + vec3(0.5);

        // Sample the 3D volume.
        float value = texture(volume, uv).r;

        // Convert density to alpha
        float alpha = value * density * step;

        // Front-to-back compositing.
        float contribution = alpha * (1.0 - accumulatedAlpha);

        // Simple grayscale volume.
        accumulatedColor += vec3(value) * contribution;

        accumulatedAlpha += contribution;


        // Once opaque, stop marching.
        if (accumulatedAlpha >= 0.99)
			break;


        // Move through the volume.
        samplePos += rayDir * step;
    }


    // Discard empty rays.
    if (accumulatedAlpha <= 0.001)
        discard;

    FragColor = vec4(accumulatedColor, accumulatedAlpha);
}