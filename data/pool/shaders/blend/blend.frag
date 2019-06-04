#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float blend;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	FragColor = mix(texture(texture1, TexCoords), texture(texture2, TexCoords), blend);
} 