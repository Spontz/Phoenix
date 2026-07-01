#type vertex
#version 440 core
layout (location = 0) in int Type;
layout (location = 1) in int ID;
layout (location = 2) in vec3 InitPosition;
layout (location = 3) in vec3 Randomness;
layout (location = 4) in vec4 InitColor;
layout (location = 5) in float Life;

uniform mat4 m4ViewModel; // view x Model matrix
uniform float fTime;

out VS_OUT
{
	flat int 	ID;
	vec4		Color;
	vec4		Position;
} vs_out;

void main(void)
{
	vs_out.ID = ID;
	vs_out.Color = InitColor;
	vec3 newPos = InitPosition + Randomness;
	vs_out.Position = m4ViewModel * vec4(newPos, 1.0);
}


#type geometry
#version 440 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 m4Projection;
uniform float fParticleSize;

// Info from the VS
in VS_OUT
{
	flat int	ID;
	vec4		Color;
	vec4		Position;
} gs_in[];

// Info sent to FS
out GS_OUT
{
	flat int	ID;
	vec4		Color;
	vec2		TexCoord;
} gs_out;

void main()
{
	gs_out.ID = gs_in[0].ID;
	gs_out.Color = gs_in[0].Color;

	vec4 P = gs_in[0].Position;

	// a: left-bottom 
	vec2 va = P.xy + vec2(-0.5, -0.5) * fParticleSize;
	gl_Position = m4Projection * vec4(va, P.zw);
	gs_out.TexCoord = vec2(0.0, 0.0);
	EmitVertex();

	// b: left-top
	vec2 vb = P.xy + vec2(-0.5, 0.5) * fParticleSize;
	gl_Position = m4Projection * vec4(vb, P.zw);
	gs_out.TexCoord = vec2(0.0, 1.0);
	EmitVertex();

	// d: right-bottom
	vec2 vd = P.xy + vec2(0.5, -0.5) * fParticleSize;
	gl_Position = m4Projection * vec4(vd, P.zw);
	gs_out.TexCoord = vec2(1.0, 0.0);
	EmitVertex();

	// c: right-top
	vec2 vc = P.xy + vec2(0.5, 0.5) * fParticleSize;
	gl_Position = m4Projection * vec4(vc, P.zw);
	gs_out.TexCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

#type fragment
#version 440 core
layout (location = 0) out vec4 FragColor;

uniform sampler2D partTexture;

in GS_OUT
{
	flat int	ID;
	vec4		Color;
	vec2		TexCoord;
} fs_in;


void main(void)
{
	FragColor = texture(partTexture, fs_in.TexCoord) * fs_in.Color;
}
