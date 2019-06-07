#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec4 particlecolor;

// Ouput data
out vec4 color;

uniform sampler2D myTextureSampler;

void main(){
	// Output color = color of the texture at the specified UV
	//color = texture( myTextureSampler, UV ) * particlecolor;
	color = vec4(1.0, 0.0, 0.0, 1.0);

}