#version 330 core
out vec4 FragColor;

uniform float blend;
uniform vec4 ColorOrigin;
uniform vec4 ColorDestiny;

void main()
{
	FragColor = mix(ColorOrigin, ColorDestiny, blend);
} 