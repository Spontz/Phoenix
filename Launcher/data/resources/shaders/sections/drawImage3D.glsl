#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

//uniforms
uniform sampler2D mainScreen;	// Main screen

uniform vec3		camPos;		// Camera position
uniform vec3		camDir;		// Camera direction

// Clouds container
uniform sampler3D	volume;		// Clouds volume dataset
uniform vec3 CloudsPos;
uniform vec3 CloudsSize;



vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 rayDir) {
	vec3 t0 = (boundsMin - rayOrigin)/rayDir;
	vec3 t1 = (boundsMax - rayOrigin)/rayDir;
	vec3 tmin = min(t0, t1);
	vec3 tmax = max(t0, t1);
	
	float dstA = max(max(tmin.x, tmin.y), tmin.z);
	float dstB = min(tmax.x, min(tmax.y, tmax.z));
	
	// Case 1: ray intersects box from outside (0 <= dstA <= dstB)
	// dstA is dst to nearest intersection, dstB dst to far intersection
	
	// Case 2: ray intersects box from inside (dstA < 0 < dstB)
	// dstA is the dst to intersection behind the ray, dstB is dst to forward intersection
	
	// Case 3: ray misses box (dstA > destB)
	
	float dstToBox = max (0, dstA);
	float dstInsideBox = max (0, dstB - dstToBox);
	return vec2(dstToBox, dstInsideBox);	
}

void main()
{
	// Calculate Cloud Box Bounds
	vec3 BoundsMin = CloudsPos - CloudsSize/2.0;
	vec3 BoundsMax = CloudsPos + CloudsSize/2.0;

	vec4 col = texture(mainScreen, TexCoords);
	vec3 rayOrigin = camPos;
	vec3 rayDir = camDir;
	
	vec2 rayBoxInfo = rayBoxDst (BoundsMin, BoundsMax, rayOrigin, rayDir);
	float dstToBox = rayBoxInfo.x;
	float dstInsideBox = rayBoxInfo.y;
	
	bool rayHitBox = (dstInsideBox > 0);
	if (!rayHitBox)
		col = vec4(1,0,0,1);
	
	FragColor = col;
}