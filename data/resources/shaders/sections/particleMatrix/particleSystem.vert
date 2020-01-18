#version 440 core
layout (location = 0) in vec3 position;
layout (location = 1) in int ID;

uniform float gTime;
uniform mat4 gPVM;
uniform float gNumParticles;

out vec4 particle_color;

void main(void)
{
	//particle_color = vec4(0.6, 0.8, 0.8, 1.0) * (smoothstep(-10.0, 10.0, position.z) * 0.6 + 0.4);
	particle_color = vec4(1.0, 1.0, 1.0, 1.0);
	
	//vec3 pos2 = position;
	//pos2.z += sin(gTime);
	float sphere = 2.0 * 3.1415 * (ID / gNumParticles);
	vec3 pos2 = vec3(0.5 * sin(sphere), 1.0 * sin(gTime), 0.5 * cos(sphere));
	
	//gl_Position = gPVM * vec4(position, 1.0);
	gl_Position = gPVM * vec4(pos2, 1.0);
}