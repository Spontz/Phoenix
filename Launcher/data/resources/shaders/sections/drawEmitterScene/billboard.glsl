#type vertex
#version 440 core

layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Color;
layout (location = 4) in int Type;

uniform mat4 model;
uniform mat4 view;

out vec3 Color0;
flat out int Type0;
#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2
void main()
{
	Color0 = Color;
	Type0 = Type;
	if (Type == PARTICLE_TYPE_EMITTER)
		gl_Position = view * model * vec4(Position, 1.0);
	else
		gl_Position = view * vec4(Position, 1.0);
}


#type geometry
#version 440 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 projection;
uniform float fParticleSize;

// Info from the VS
in vec3 Color0[];
flat in int Type0[];

// Info sent to FS
out vec2 TexCoord;
out vec3 Color1;
flat out int Type1;

void main()
{
	Color1 = Color0[0];
	Type1 = Type0[0];
	
	vec4 P = gl_in[0].gl_Position;

	// a: left-bottom 
	vec2 va = P.xy + vec2(-0.5, -0.5) * fParticleSize;
	gl_Position = projection * vec4(va, P.zw);
	TexCoord = vec2(0.0, 0.0);
	EmitVertex();

	// b: left-top
	vec2 vb = P.xy + vec2(-0.5, 0.5) * fParticleSize;
	gl_Position = projection * vec4(vb, P.zw);
	TexCoord = vec2(0.0, 1.0);
	EmitVertex();

	// d: right-bottom
	vec2 vd = P.xy + vec2(0.5, -0.5) * fParticleSize;
	gl_Position = projection * vec4(vd, P.zw);
	TexCoord = vec2(1.0, 0.0);
	EmitVertex();

	// c: right-top
	vec2 vc = P.xy + vec2(0.5, 0.5) * fParticleSize;
	gl_Position = projection * vec4(vc, P.zw);
	TexCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

#type fragment
#version 440 core

uniform sampler2D partTexture;

in vec2 TexCoord;
in vec3 Color1;
flat in int Type1;

out vec4 FragColor;


void main()
{
	FragColor = texture(partTexture, TexCoord) * vec4(Color1.rgb, 1.0f);
}