#version 330 core
out vec4 FragColor;

in vec3 Color;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;

void main()
{   
	//FragColor = vec4(Color,1);
    FragColor = texture(texture_diffuse1, TexCoord);
	
}