#version 440 core
layout (location = 0) out vec4 FragColor;

uniform sampler2D partTexture;

in vec3 Color1;
in vec2 TexCoord;


void main(void)
{
	FragColor = texture(partTexture, TexCoord) * vec4(Color1.rgb, 1.0f);
}