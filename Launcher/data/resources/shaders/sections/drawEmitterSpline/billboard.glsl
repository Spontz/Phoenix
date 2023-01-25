#type vertex
#version 450 core

layout (location = 0) in vec3 Position;
layout (location = 4) in vec3 Color;


uniform mat4 model;
uniform mat4 view;

out VS_OUT
{
	vec4		Position;
	vec3		Color;
} vs_out;

void main()
{
	vs_out.Color = Color;
	
//	vs_out.Position = view * model * vec4(Position, 1.0);
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
} gs_in[];

// Info sent to FS
out GS_OUT
{
	vec2		TexCoord;
	vec3		Color;
} gs_out;

void main()
{
	gs_out.Color	= gs_in[0].Color;
		
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
} fs_in;

out vec4 FragColor;

void main()
{
	FragColor = texture(partTexture, fs_in.TexCoord) * vec4(fs_in.Color.rgb, 1.0f);
}