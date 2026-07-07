#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(aPos, 1.0);
}  

#type fragment
#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
	FragColor = vec4(color, 1.0);
}