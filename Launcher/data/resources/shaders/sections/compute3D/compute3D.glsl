#type compute
#version 450

layout(
	local_size_x = 8,
	local_size_y = 8,
	local_size_z = 8
) in;

layout(r16f, binding = 0) uniform image3D volume;

// Generates a volumetric sphere
void main()
{
	ivec3 p = ivec3(gl_GlobalInvocationID);

	ivec3 size = imageSize(volume);

	if (any(greaterThanEqual(p, size)))
		return;

	vec3 uv = (vec3(p) + 0.5) / vec3(size);

	float d = length( uv - vec3(0.5));

	float value = smoothstep(0.35, 0.30, d);

	imageStore(volume, p, vec4(value));
}