#type vertex
#version 450 core

layout (location = 0) in int Type;
layout (location = 1) in int ID;
layout (location = 2) in vec3 InitPosition;
layout (location = 3) in vec3 Position;
layout (location = 4) in vec3 Randomness;
layout (location = 5) in vec3 Rotation;
layout (location = 6) in vec3 InitColor;
layout (location = 7) in vec3 Color;
layout (location = 8) in float Age;
layout (location = 9) in float Life;

out VS_OUT
{
	flat int		Type;
	flat int		ID;
	vec3			InitPosition;
	vec3			Position;
	vec3			Randomness;
	vec3			Rotation;
	vec3			InitColor;
	vec3			Color;
	float			Age;
	float			Life;
} vs_out;

void main()
{
	vs_out.Type			= Type;
	vs_out.ID			= ID;
	vs_out.InitPosition	= InitPosition;
	vs_out.Position		= Position;
	vs_out.Randomness	= Randomness;
	vs_out.Rotation		= Rotation;
	vs_out.InitColor	= InitColor;
	vs_out.Color		= Color;
	vs_out.Age			= Age;
	vs_out.Life			= Life;
}

#type geometry
#version 450 core

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

// Info from the VS
in VS_OUT
{
	flat int		Type;
	flat int		ID;
	vec3			InitPosition;
	vec3			Position;
	vec3			Randomness;
	vec3			Rotation;
	vec3			InitColor;
	vec3			Color;
	float			Age;
	float			Life;
} gs_in[];

// Info sent to FS
flat out int		o_Type;
flat out int		o_ID;
out vec3			o_InitPosition;
out vec3			o_Position;
out vec3			o_Randomness;
out vec3			o_Rotation;
out vec3			o_InitColor;
out vec3			o_Color;
out float			o_Age;
out float			o_Life;

uniform float u_fDeltaTime;
uniform float u_fTime;
uniform vec3 u_v3Force;
uniform vec3 u_v3Color;
uniform float u_fRamndomness;
uniform sampler1D u_iRandomTexture;
uniform float u_fParticleLifetime;
uniform uint u_uiNumMaxParticles;
uniform uint u_uiNumParticlesPerEmitter;

#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2

uniform mat4 u_m4Model;

// Get a random number, used for calculate the new direction
vec3 GetRandomDir(float TexCoord)
{
	vec3 Dir = texture(u_iRandomTexture, TexCoord).xyz;
	Dir -= vec3(0.5, 0.5, 0.5); // Since all tex coordinates goes from 0 to 1, we substract 0.5 so all values will go between -0.5 and 0.5
	return Dir;
}

void main()
{
	float fParticleID = float(gs_in[0].ID)/float(u_uiNumParticlesPerEmitter); // Get particle ID between 0 and 1
	
	// Calculate the age of the particle, given the current time
	float IntervalEmission = gs_in[0].Life/float(u_uiNumParticlesPerEmitter); // time between particles = particleLife/numParticles
	float emissionTime = float(gs_in[0].ID) * IntervalEmission; // Time that the particle should be emitted
	float RelativeAge = mod(u_fTime+emissionTime, gs_in[0].Life);
	float AbsoluteAge =  RelativeAge/ gs_in[0].Life; // This gives a number between 0 to 1 with the real age of the particle
	
	if (gs_in[0].Type == PARTICLE_TYPE_EMITTER) {
		// Draw the Emitter
		o_Type = PARTICLE_TYPE_EMITTER;
		o_ID = gs_in[0].ID;
		o_InitPosition = gs_in[0].InitPosition;
		o_InitColor = gs_in[0].InitColor;
		o_Position = gs_in[0].InitPosition;
		o_Randomness = gs_in[0].Randomness;
		o_Rotation = gs_in[0].Rotation;
		o_Color = u_v3Color;
		o_Age = RelativeAge;
		o_Life = gs_in[0].Life;
		
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
	else if (gs_in[0].Type == PARTICLE_TYPE_SHELL) {
	
		// Draw the Normal Particle
		o_Type = PARTICLE_TYPE_SHELL;
		o_ID = gs_in[0].ID;
		o_InitPosition = gs_in[0].InitPosition;
		o_Rotation = gs_in[0].Rotation;
		o_Age = RelativeAge;
		o_Life = gs_in[0].Life;
		
		// If new age (RelativeAge) is below the previous age means that the particle has been regenerated, then we capture the new color
		if(RelativeAge < gs_in[0].Age) {
			o_InitColor = u_v3Color;
			o_Color = u_v3Color * (1-AbsoluteAge);
			vec3 newRandom = u_fRamndomness*GetRandomDir(fParticleID);
			o_Randomness = newRandom;
			o_Position = gs_in[0].InitPosition + (vec3(2,0,0) - gs_in[0].InitPosition) * AbsoluteAge + newRandom;
			
		}		
		else {
			o_InitColor = gs_in[0].InitColor;
			o_Color = gs_in[0].InitColor * (1-AbsoluteAge);
			o_Randomness = gs_in[0].Randomness;
			o_Position = gs_in[0].InitPosition + (vec3(2,0,0) - gs_in[0].InitPosition) * AbsoluteAge + gs_in[0].Randomness;
		}

		
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
}


#type fragment
#version 450 core

void main()
{
}