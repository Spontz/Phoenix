#type compute
#version 460 core

layout(
    local_size_x = 8,
    local_size_y = 8,
    local_size_z = 8
) in;


/*
 * Output volume.
 *
 * R16F:
 *     0.0 = empty
 *     1.0 = solid
 */
layout(r16f, binding = 0) uniform image3D volume;


/*
 * Number of Menger iterations.
 *
 * 1 = basic Menger Sponge
 * 2 = 20^2 cells
 * 3 = 20^3 cells
 * ...
 */
const int iterations = 3;
//uniform int iterations;


/*
 * Optional animation.
 *
 * Keep at 0 initially.
 */
uniform float time;


/*
 * Evaluate one Menger iteration.
 *
 * Returns:
 *
 *     true  -> this region survives
 *     false -> this region is removed
 */
bool mengerIteration(vec3 p)
{
    /*
     * Convert the coordinates from [-0.5, +0.5]
     * to [0, 1].
     */
    vec3 q = p + vec3(0.5);

    /*
     * Keep testing the base-3 digits.
     *
     * At each iteration we determine the
     * position inside a 3x3x3 subdivision.
     */
    for (int i = 0; i < 32; ++i)
    {
        if (i >= iterations)
            break;

        /*
         * Scale current cell into a 3x3x3 grid.
         */
        q *= 3.0;

        /*
         * Current digit in base 3.
         */
        vec3 cell =
            floor(q);

        /*
         * Position of the current cell inside
         * the 3x3x3 subdivision:
         *
         *     x = 0..2
         *     y = 0..2
         *     z = 0..2
         */
        int x = int(mod(cell.x, 3.0));
        int y = int(mod(cell.y, 3.0));
        int z = int(mod(cell.z, 3.0));

        /*
         * Menger removal rule:
         *
         * Remove the voxel if at least two axes
         * are in the middle cell (1).
         *
         * Examples:
         *
         *     (1,1,0) -> removed
         *     (1,0,1) -> removed
         *     (0,1,1) -> removed
         *     (1,1,1) -> removed
         *
         * Otherwise the cell survives.
         */
        int middleCount = 0;

        if (x == 1)
            middleCount++;

        if (y == 1)
            middleCount++;

        if (z == 1)
            middleCount++;

        if (middleCount >= 2)
            return false;

        /*
         * Keep only the fractional part so the next
         * iteration works on the selected sub-cell.
         */
        q = fract(q);
    }

    return true;
}


void main()
{
    ivec3 voxel =
        ivec3(gl_GlobalInvocationID);

    ivec3 size =
        imageSize(volume);

    /*
     * Bounds check.
     */
    if (any(greaterThanEqual(voxel, size)))
        return;


    /*
     * Convert voxel coordinate to normalized [0,1].
     */
    vec3 uv =
        (vec3(voxel) + vec3(0.5)) /
        vec3(size);


    /*
     * Convert to [-0.5,+0.5].
     */
    vec3 p =
        uv - vec3(0.5);


    /*
     * Evaluate Menger Sponge.
     */
    bool solid =
        mengerIteration(p);


    /*
     * Optional tiny animation.
     *
     * Disabled by default.
     *
     * You can remove this block completely if
     * you don't want animation.
     */
    if (time != 0.0)
    {
        /*
         * Very subtle movement to prove that the
         * compute shader is being regenerated.
         *
         * This does NOT modify the topology of the
         * Menger Sponge, only the coordinate system.
         */
        float angle =
            time * 0.2;

        float c = cos(angle);
        float s = sin(angle);

        vec3 rotated;

        rotated.x =
            p.x * c -
            p.z * s;

        rotated.y =
            p.y;

        rotated.z =
            p.x * s +
            p.z * c;

        solid =
            mengerIteration(rotated);
    }


    /*
     * Write density.
     */
    imageStore(
        volume,
        voxel,
        vec4(
            solid ? 1.0 : 0.0
        )
    );
}