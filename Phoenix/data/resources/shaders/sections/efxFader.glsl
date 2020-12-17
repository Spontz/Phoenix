#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 FragColor;

uniform float blend;
uniform vec4 ColorOrigin;
uniform vec4 ColorDestiny;

void main()
{
	FragColor = mix(ColorOrigin, ColorDestiny, blend);
} 