#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
	vec2 v_texcoord = vec2(TexCoords.s, 1.0 - TexCoords.t);
    vec3 col = texture(screenTexture, v_texcoord).rgb;
    FragColor = vec4(col, 1.0);
}