#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float alpha;
uniform sampler2D screenTexture;

void main()
{
    vec3 col = texture(screenTexture, TexCoords).rgb*(alpha);
    FragColor = vec4(col, alpha);
} 