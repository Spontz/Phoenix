#type vertex
#version 450 core
layout (location = 0) in int Type;
layout (location = 1) in int ID;
layout (location = 2) in vec3 InitPosition;
layout (location = 3) in vec3 Randomness;
layout (location = 4) in vec4 InitColor;
layout (location = 5) in float Life;

uniform mat4 m4ViewModel; // view x Model matrix
uniform float fTime;
uniform int iNumParticlesPerEmitter;
uniform float fRandomnessFactor;

out VS_OUT
{
	vec4	Color;
	vec4	Position;
} vs_out;

#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2

void main(void)
{
	// Get particle ID between 0 and 1
	float fParticleID = float(ID)/float(iNumParticlesPerEmitter);
	// Calculate the age of the particle, given the current time
	float IntervalEmission = Life/float(iNumParticlesPerEmitter); // time between particles = particleLife/numParticles
	float emissionTime = float(ID) * IntervalEmission; // Time that the particle should be emitted
	// RelativeAge: Value between 0 to "Life"
	float RelativeAge = mod(fTime+emissionTime, Life);
	// AbsoluteAge: Value from 0 to 1 with the life of the particle
	float AbsoluteAge =  RelativeAge/Life;
	
	vec3 finalPosition = InitPosition;
	vec4 finalColor = InitColor;
	vec3 finalRandom = Randomness * fRandomnessFactor;
	
	if (Type == PARTICLE_TYPE_SHELL) {
		// Example of fire effect:
		finalColor = mix(InitColor, vec4(1,1,0,1)*(1-AbsoluteAge), AbsoluteAge);
		finalPosition = InitPosition + (vec3(0,1,0)*AbsoluteAge)+ finalRandom;
	}
	
	vs_out.Color = finalColor;
	vs_out.Position = m4ViewModel * vec4(finalPosition, 1.0);
}


#type geometry
#version 450 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 m4Projection;
uniform float fParticleSize;

// Info from the VS
in VS_OUT
{
	vec4	Color;
	vec4	Position;
} gs_in[];

// Info sent to FS
out GS_OUT
{
	vec2	TexCoord;
	vec4	Color;
} gs_out;

void main()
{
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
#version 450 core
layout (location = 0) out vec4 FragColor;

uniform sampler2D partTexture;

in GS_OUT
{
	vec2		TexCoord;
	vec4		Color;
} fs_in;


void main(void)
{
	FragColor = texture(partTexture, fs_in.TexCoord) * fs_in.Color;
}
