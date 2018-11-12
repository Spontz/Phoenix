#version 430

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 o_position;
out vec3 o_normal;
out vec2 TexCoords;
	
uniform mat4 world;
uniform mat4 viewProj;
uniform mat3 normalMatrix;
	
void main()
{
	o_position = vec3(world * vec4(aPos, 1.0f));
    o_normal   = normalMatrix * aNormal;
    TexCoords = aTexCoords;
	
    gl_Position = viewProj * world * vec4(aPos, 1.0f);
}