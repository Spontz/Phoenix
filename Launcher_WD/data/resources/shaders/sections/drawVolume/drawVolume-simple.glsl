#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 LocalPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
	LocalPos = aPos;
	gl_Position = projection * view * model * vec4(aPos, 1.0); 
}

#type fragment
#version 330 core

in vec2 TexCoords;
in vec3 LocalPos;
out vec4 FragColor;

void main() {
	FragColor = vec4(LocalPos.xyz+0.5,1.0);
}