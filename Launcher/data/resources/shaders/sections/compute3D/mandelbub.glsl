#type compute
#version 450

layout(
    local_size_x = 8,
    local_size_y = 8,
    local_size_z = 8
) in;

layout(r16f, binding = 0) uniform image3D volume;


// ============================================================
// MANDELBULB CONFIGURATION
// ============================================================

// Number of iterations.
const int MB_ITERATIONS = 12;

// Mandelbulb power.
// Classic Mandelbulb = 8.0
//
// This is one of the best parameters to animate.
const float MB_POWER = 8.0;

// Overall fractal scale.
const float MB_SCALE = 1.0;

// Escape radius.
const float MB_BAILOUT = 2.0;


// ============================================================
// ANIMATION
// ============================================================

// Time.
//
// Set this to the value coming from your engine when you
// want animation.
//
// For now it is a constant so the shader works standalone.
const float ANIMATION_TIME = 0.0;
//uniform float ANIMATION_TIME;


// Amount by which the power is animated.
//
// Example:
//
// MB_POWER = 8.0
// ANIMATION_POWER = 2.0
//
// gives a power oscillating between 6 and 10.
const float ANIMATION_POWER = 1.0;


// ============================================================
// MANDELBULB DISTANCE ESTIMATOR
// ============================================================

float mandelbulbDE(vec3 p)
{
    vec3 z = p;

    float dr = 1.0;
    float r = 0.0;

    // Animate the Mandelbulb power.
    float power =
        MB_POWER +
        sin(ANIMATION_TIME) * ANIMATION_POWER;


    for (int i = 0; i < MB_ITERATIONS; ++i)
    {
        r = length(z);

        if (r > MB_BAILOUT)
            break;


        // ----------------------------------------------------
        // Cartesian -> spherical coordinates
        // ----------------------------------------------------

        float theta =
            acos(
                clamp(
                    z.z / max(r, 0.00001),
                    -1.0,
                    1.0
                )
            );

        float phi = atan(z.y, z.x);


        // ----------------------------------------------------
        // Derivative
        // ----------------------------------------------------

        dr =
            pow(r, power - 1.0) *
            power *
            dr +
            1.0;


        // ----------------------------------------------------
        // Mandelbulb power operation
        // ----------------------------------------------------

        float zr = pow(r, power);

        float newTheta = theta * power;
        float newPhi   = phi   * power;


        z = zr * vec3(
            sin(newTheta) * cos(newPhi),
            sin(newTheta) * sin(newPhi),
            cos(newTheta)
        );


        // ----------------------------------------------------
        // Mandelbrot-style addition
        // ----------------------------------------------------

        z += p;
    }


    // --------------------------------------------------------
    // Distance estimator
    // --------------------------------------------------------

    return 0.5 * log(r) * r / dr;
}


// ============================================================
// MAIN
// ============================================================

void main()
{
    ivec3 p = ivec3(gl_GlobalInvocationID);

    ivec3 size = imageSize(volume);


    // --------------------------------------------------------
    // Bounds check
    // --------------------------------------------------------

    if (any(greaterThanEqual(p, size)))
        return;


    // --------------------------------------------------------
    // Voxel coordinates -> [0,1]
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
    // Preserve aspect ratio.
    //
    // This prevents the Mandelbulb from being stretched if
    // the 3D texture isn't perfectly cubic.
    // --------------------------------------------------------

    float minSize = float(
        min(
            size.x,
            min(size.y, size.z)
        )
    );

    vec3 voxelScale =
        vec3(size) / minSize;

    pos *= voxelScale;


    // --------------------------------------------------------
    // Mandelbulb scale
    // --------------------------------------------------------

    pos /= MB_SCALE;


    // --------------------------------------------------------
    // Distance estimation
    // --------------------------------------------------------

    float d =
        mandelbulbDE(pos);


    // --------------------------------------------------------
    // Convert distance into density.
    // --------------------------------------------------------

    const float SURFACE_THICKNESS = 0.025;

    float density =
        1.0 -
        smoothstep(
            0.0,
            SURFACE_THICKNESS,
            abs(d)
        );


    // --------------------------------------------------------
    // Solid interior
    // --------------------------------------------------------

    if (d < 0.0)
        density = 1.0;


    // --------------------------------------------------------
    // Write voxel
    // --------------------------------------------------------

    imageStore(
        volume,
        p,
        vec4(density)
    );
}