#type compute
#version 450

layout(
    local_size_x = 8,
    local_size_y = 8,
    local_size_z = 8
) in;

layout(r16f, binding = 0) uniform image3D volume;


// ============================================================
// CONFIGURATION
// ============================================================

// Radius of each metaball.
const float BALL_RADIUS = 0.18;


// Strength of each metaball field.
const float BALL_STRENGTH = 1.0;


// ------------------------------------------------------------
// ISO VALUE
// ------------------------------------------------------------
//
// This is the threshold used to determine the metaball
// surface.
//
// Lower values:
//      -> bigger metaballs
//      -> easier to merge
//
// Higher values:
//      -> smaller metaballs
//      -> harder to merge
//
// Try:
//
//     0.20
//     0.30
//     0.40
//     0.50
//
const float ISO_VALUE = 0.35;


// ------------------------------------------------------------
// ISO SMOOTHNESS
// ------------------------------------------------------------
//
// Controls how soft the transition around the ISO surface is.
//
// 0.0 -> hard surface
// 0.02 -> slightly soft
// 0.05 -> smooth
//
const float ISO_SMOOTHNESS = 0.02;


// ------------------------------------------------------------
// FIELD POWER
// ------------------------------------------------------------
//
// Controls how quickly the metaball field falls off.
//
// Lower:
//      larger influence
//      easier merging
//
// Higher:
//      more defined spheres
//      less influence
//
const float FIELD_POWER = 4.0;


// ============================================================
// ANIMATION
// ============================================================

//const float TIME = 0.0;
uniform float TIME;

const float ANIMATION_SPEED = 1.0;


// ============================================================
// METABALL POSITIONS
// ============================================================

vec3 getBall1(float t)
{
    return vec3(
        sin(t * 0.8) * 0.45,
        cos(t * 0.6) * 0.30,
        sin(t * 0.5) * 0.35
    );
}


vec3 getBall2(float t)
{
    return vec3(
        cos(t * 0.7) * 0.45,
        sin(t * 0.9) * 0.35,
        cos(t * 0.4) * 0.30
    );
}


vec3 getBall3(float t)
{
    return vec3(
        sin(t * 0.5 + 2.0) * 0.40,
        cos(t * 0.8 + 1.0) * 0.40,
        sin(t * 0.7 + 3.0) * 0.30
    );
}


// ============================================================
// METABALL FIELD
// ============================================================

float sphereField(
    vec3 p,
    vec3 center,
    float radius
)
{
    float d = length(p - center);

    // Normalize distance by sphere radius.
    float q = d / radius;

    q = max(q, 0.00001);

    // Inverse power field.
    //
    // At the center:
    //
    //     field ~= BALL_STRENGTH
    //
    // As distance increases:
    //
    //     field -> 0
    //
    return
        BALL_STRENGTH /
        (1.0 + pow(q, FIELD_POWER));
}


// ============================================================
// MAIN
// ============================================================

void main()
{
    ivec3 p =
        ivec3(gl_GlobalInvocationID);

    ivec3 size =
        imageSize(volume);


    // --------------------------------------------------------
    // Bounds check
    // --------------------------------------------------------

    if (any(greaterThanEqual(p, size)))
        return;


    // --------------------------------------------------------
    // Voxel -> [0,1]
    // --------------------------------------------------------

    vec3 uv =
        (vec3(p) + 0.5) /
        vec3(size);


    // --------------------------------------------------------
    // [0,1] -> [-1,+1]
    // --------------------------------------------------------

    vec3 pos =
        uv * 2.0 -
        1.0;


    // --------------------------------------------------------
    // Preserve aspect ratio
    // --------------------------------------------------------

    float minSize =
        float(
            min(
                size.x,
                min(size.y, size.z)
            )
        );

    vec3 voxelScale =
        vec3(size) / minSize;

    pos *= voxelScale;


    // --------------------------------------------------------
    // Animation
    // --------------------------------------------------------

    float t =
        TIME *
        ANIMATION_SPEED;


    // --------------------------------------------------------
    // Metaball positions
    // --------------------------------------------------------

    vec3 ball1 =
        getBall1(t);

    vec3 ball2 =
        getBall2(t);

    vec3 ball3 =
        getBall3(t);


    // --------------------------------------------------------
    // Calculate combined field
    // --------------------------------------------------------
    //
    // IMPORTANT:
    //
    // The three fields are added BEFORE applying ISO_VALUE.
    //
    // This is what produces the metaball fusion.
    //
    // --------------------------------------------------------

    float field = 0.0;

    field +=
        sphereField(
            pos,
            ball1,
            BALL_RADIUS
        );

    field +=
        sphereField(
            pos,
            ball2,
            BALL_RADIUS
        );

    field +=
        sphereField(
            pos,
            ball3,
            BALL_RADIUS
        );


    // ========================================================
    // APPLY ISO VALUE
    // ========================================================
    //
    // The surface exists where:
    //
    //              field = ISO_VALUE
    //
    // Inside:
    //
    //              field > ISO_VALUE
    //
    // Outside:
    //
    //              field < ISO_VALUE
    //
    // ========================================================

    float density;

    if (ISO_SMOOTHNESS <= 0.0)
    {
        // Hard isosurface.

        density =
            field >= ISO_VALUE
            ? 1.0
            : 0.0;
    }
    else
    {
        // Smooth isosurface.

        density =
            smoothstep(
                ISO_VALUE - ISO_SMOOTHNESS,
                ISO_VALUE + ISO_SMOOTHNESS,
                field
            );
    }


    // --------------------------------------------------------
    // Store final density
    // --------------------------------------------------------

    imageStore(
        volume,
        p,
        vec4(density)
    );
}