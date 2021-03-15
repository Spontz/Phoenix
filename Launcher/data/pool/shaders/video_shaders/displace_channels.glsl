#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float displace;

void main()
{
	vec3 col;
	col.r = texture(screenTexture, vec2(TexCoords.s+displace, TexCoords.t)).r;
	col.g = texture(screenTexture, vec2(TexCoords.s-displace, TexCoords.t)).g;
	col.b = texture(screenTexture, TexCoords).b;

    FragColor = vec4(col, 1.0);
}