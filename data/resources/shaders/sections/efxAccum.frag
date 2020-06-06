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