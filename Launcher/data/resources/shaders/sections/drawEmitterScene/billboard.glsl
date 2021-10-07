#type vertex
#version 450 core

layout (location = 0) in int Type;
layout (location = 1) in vec3 Position;
layout (location = 3) in vec3 Color;


uniform mat4 model;
uniform mat4 view;

out VS_OUT
{
	vec4		Position;
	vec3		Color;
	flat int	Type;
} vs_out;

#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2
void main()
{
	vs_out.Color = Color;
	vs_out.Type = Type;
	
	if (Type == PARTICLE_TYPE_EMITTER)
		vs_out.Position = view * model * vec4(Position, 1.0);
	if (Type == PARTICLE_TYPE_SHELL)
		vs_out.Position = view * vec4(Position, 1.0);
}


#type geometry
#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 projection;
uniform float fParticleSize;

// Info from the VS
in VS_OUT
{
	vec4		Position;
	vec3		Color;
	flat int	Type;
} gs_in[];

// Info sent to FS
out GS_OUT
{
	vec2		TexCoord;
	vec3		Color;
	flat int	Type;
} gs_out;

void main()
{
	gs_out.Color	= gs_in[0].Color;
	gs_out.Type		= gs_in[0].Type;
	
	vec4 P = gs_in[0].Position;

	// a: left-bottom 
	vec2 va = P.xy + vec2(-0.5, -0.5) * fParticleSize;
	gl_Position = projection * vec4(va, P.zw);
	gs_out.TexCoord = vec2(0.0, 0.0);
	EmitVertex();

	// b: left-top
	vec2 vb = P.xy + vec2(-0.5, 0.5) * fParticleSize;
	gl_Position = projection * vec4(vb, P.zw);
	gs_out.TexCoord = vec2(0.0, 1.0);
	EmitVertex();

	// d: right-bottom
	vec2 vd = P.xy + vec2(0.5, -0.5) * fParticleSize;
	gl_Position = projection * vec4(vd, P.zw);
	gs_out.TexCoord = vec2(1.0, 0.0);
	EmitVertex();

	// c: right-top
	vec2 vc = P.xy + vec2(0.5, 0.5) * fParticleSize;
	gl_Position = projection * vec4(vc, P.zw);
	gs_out.TexCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

#type fragment
#version 450 core

uniform sampler2D partTexture;

in GS_OUT
{
	vec2		TexCoord;
	vec3		Color;
	flat int	Type;
} fs_in;

out vec4 FragColor;

void main()
{
	FragColor = texture(partTexture, fs_in.TexCoord) * vec4(fs_in.Color.rgb, 1.0f);
}