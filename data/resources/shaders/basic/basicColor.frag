#version 330 core
out vec4 FragColor;

in vec3 Color;
in vec2 TexCoord;


void main()
{   
	FragColor = vec4(Color,1);
}