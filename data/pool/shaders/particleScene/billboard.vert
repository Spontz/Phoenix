#version 330

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Color;
layout (location = 4) in int Type;

uniform mat4 model;

out vec3 Color0;
flat out int Type0;

void main()
{
	Color0 = Color;
	Type0 = Type;
	gl_Position = model * vec4(Position, 1.0);
}