#type vertex
#version 330

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Color;
layout (location = 4) in int Type;

uniform mat4 model;

out vec3 Color0;
flat out int Type0;

void main()
{
	Color0 = Color;
	Type0 = Type;
	gl_Position = model * vec4(Position, 1.0);
}

#type geometry
#version 330

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 gVP;
uniform vec3 gCameraPos;
uniform float fParticleSize;

// Info from the VS
in vec3 Color0[];
flat in int Type0[];

// Info sent to FS
out vec3 Color1;
flat out int Type1;

out vec2 TexCoord;

void main()
{
	Color1 = Color0[0];
	Type1 = Type0[0];
	
	vec3 Pos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(gCameraPos - Pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(toCamera, up) * fParticleSize;

	Pos -= right;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 0.0);
	EmitVertex();

	Pos.y += fParticleSize;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 1.0);
	EmitVertex();

	Pos.y -= fParticleSize;
	Pos += right;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 0.0);
	EmitVertex();

	Pos.y += fParticleSize;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}                                                                                   


#type fragment
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