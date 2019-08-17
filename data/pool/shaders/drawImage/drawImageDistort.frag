#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec3 distortion;

void main()
{
	vec4 col = texture(screenTexture, TexCoords).rgba;
	col *= vec4(distortion, 1.0);
	FragColor = vec4(col);
}