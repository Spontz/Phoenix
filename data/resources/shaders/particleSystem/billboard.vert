#version 330

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Color;

uniform mat4 model;

out vec3 Color0;

void main()
{
	Color0 = Color;
	gl_Position = model * vec4(Position, 1.0);
}