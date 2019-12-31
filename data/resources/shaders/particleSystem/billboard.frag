#version 330

uniform sampler2D gColorMap;

in vec2 TexCoord;
in vec3 Velocity1;
in vec3 Color1;

out vec4 FragColor;

void main()
{
	FragColor = texture(gColorMap, TexCoord) * vec4(Color1.rgb, 1.0f);

/*	if ((Color1.r < 0.0) &&
		(Color1.g < 0.0) &&
		(Color1.b < 0.0)) {
	
		discard;
    }
*/
}