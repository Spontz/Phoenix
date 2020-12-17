#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 direction;

void main(void)
{
	vec2 resolution = textureSize(screenTexture, 0);


	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.411764705882353) * direction;
	vec2 off2 = vec2(3.2941176470588234) * direction;
	vec2 off3 = vec2(5.176470588235294) * direction;
	color += texture(screenTexture, TexCoords.xy) * 0.1964825501511404;
	color += texture(screenTexture, TexCoords.xy + (off1 / resolution)) * 0.2969069646728344;
	color += texture(screenTexture, TexCoords.xy - (off1 / resolution)) * 0.2969069646728344;
	color += texture(screenTexture, TexCoords.xy + (off2 / resolution)) * 0.09447039785044732;
	color += texture(screenTexture, TexCoords.xy - (off2 / resolution)) * 0.09447039785044732;
	color += texture(screenTexture, TexCoords.xy + (off3 / resolution)) * 0.010381362401148057;
	color += texture(screenTexture, TexCoords.xy - (off3 / resolution)) * 0.010381362401148057;

	FragColor = color;
}