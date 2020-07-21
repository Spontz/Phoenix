#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sourceImage;
uniform sampler2D accumImage;
uniform float sourceInfluence; // 0.0 to 1.0
uniform float accumInfluence; // 0.0 to 1.0

void main()
{
	
	vec3 source = texture(sourceImage, TexCoords).rgb;
	vec3 accum = texture(accumImage, TexCoords).rgb;

	FragColor = vec4(source * sourceInfluence  + accum * accumInfluence, 1.0);
	
	//FragColor = vec4(mix(source, accum, influence), 1.0);
}