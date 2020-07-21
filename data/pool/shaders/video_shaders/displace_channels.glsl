#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
uniform mat4 model;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float displace;

void main()
{
	vec2 v_texcoord = vec2(TexCoords.s, 1.0 - TexCoords.t);
	vec3 col;
	col.r = texture(screenTexture, vec2(v_texcoord.s+displace, v_texcoord.t)).r;
	col.g = texture(screenTexture, vec2(v_texcoord.s-displace, v_texcoord.t)).g;
	col.b = texture(screenTexture, v_texcoord).b;

    FragColor = vec4(col, 1.0);
}