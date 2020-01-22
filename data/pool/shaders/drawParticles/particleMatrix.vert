#version 440 core
layout (location = 0) in vec3 position;
layout (location = 1) in int ID;

uniform float gTime;
uniform mat4 gModel;
uniform float gNumParticles;

out int ID0;
out vec3 Color0;

void main(void)
{
	//Color0 = vec3(0.6, 0.8, 0.8) * (smoothstep(-10.0, 10.0, position.z) * 0.6 + 0.4);
	Color0 = vec3(1.0, 0.5+0.5*sin(gTime), 1.0);
	ID0 = ID;
	
	//vec3 pos2 = position;
	//pos2.z += sin(gTime);
	float sphere = 2.0 * 3.1415 * (ID / gNumParticles);
	vec3 pos2 = vec3(0.5 * sin(sphere), 1.0 * sin(gTime), 0.5 * cos(sphere));

	gl_Position = gModel * vec4(pos2, 1.0);
}