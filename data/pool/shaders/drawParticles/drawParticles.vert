#version 440 core
layout (location = 0) in vec3 Position;

uniform float gTime;
uniform mat4 gModel;
uniform float gNumParticles;

out int ID0;
out vec3 Color0;

#define PI 3.1415926535897932384626433832795

void main(void)
{
	ID0 = gl_VertexID;	// Send the vertex ID (=particle ID) to Geometry shader
	
	float zero_to_one = gl_VertexID / gNumParticles;
	float sphere = 2.0* PI * zero_to_one;
	
	// Calculate the color of the particle
	//Color0 = vec3(1.0, 0.5+0.5*sin(gTime), 1.0);
	Color0 = vec3(1+sin(((sphere-PI)/2.0)+gTime), 0.0, 0.0);//0.5+0.5*sin(gTime), 1.0);

	// Calculate the new position of the particle
	vec3 new_position = vec3(0.5 * sin(sphere), 1.0 * sin(gTime), 0.5 * cos(sphere));


	gl_Position = gModel * vec4(new_position, 1.0);
}