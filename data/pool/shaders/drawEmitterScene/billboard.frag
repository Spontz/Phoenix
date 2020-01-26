#version 330

uniform sampler2D partTexture;

in vec2 TexCoord;
in vec3 Color1;
flat in int Type1;

out vec4 FragColor;


#define PARTICLE_TYPE_EMITTER 0
#define PARTICLE_TYPE_SHELL 1

void main()
{
	
//	if (Type1 == PARTICLE_TYPE_SHELL)
//		discard;

	FragColor = texture(partTexture, TexCoord) * vec4(Color1.rgb, 1.0f);

/*	if ((Color1.r < 0.0) &&
		(Color1.g < 0.0) &&
		(Color1.b < 0.0)) {
	
		discard;
    }
*/
}