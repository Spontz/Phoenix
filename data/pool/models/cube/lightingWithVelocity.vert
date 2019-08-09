#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

smooth out vec4 vPosition;
smooth out vec4 vPrevPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 prev_projection;
uniform mat4 prev_view;
uniform mat4 prev_model;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
    
	vec3 n = aNormal;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * n);
    
	vPosition = projection * view * model * vec4(aPos, 1.0);
	vPrevPosition = prev_projection * prev_view * prev_model * vec4(aPos, 1.0);
	
    gl_Position = vPosition;
}