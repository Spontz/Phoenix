#version 330 core
out vec4 FragColor;

in vec3 Color;
in vec2 TexCoord;
uniform vec3 add_color;

void main()
{ 
	vec3 new_col = Color+add_color;
 	FragColor = vec4(new_col, 1.0);
}