#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Velocity;
layout (location = 2) in vec3 Color;
layout (location = 3) in float Age;
layout (location = 4) in int Type;

out vec3 Position0;
out vec3 Velocity0;
out vec3 Color0;
out float Age0;
out int Type0;

void main()
{
	Position0 = Position;
	Velocity0 = Velocity;
	Color0 = Color;
	Age0 = Age;
	Type0 = Type;
}