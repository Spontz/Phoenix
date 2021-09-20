#type vertex
#version 450 core

layout (location = 0) in int Type;
layout (location = 1) in int ID;
layout (location = 2) in vec3 InitPosition;
layout (location = 3) in vec3 Position;
layout (location = 4) in vec3 Randomness;
layout (location = 5) in vec3 Rotation;
layout (location = 6) in vec3 Color;
layout (location = 7) in float Age;
layout (location = 8) in float Life;

out VS_OUT
{
	flat int		Type;
	flat int		ID;
	vec3			InitPosition;
	vec3			Position;
	vec3			Randomness;
	vec3			Rotation;
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
	vs_out.Color		= Color;
	vs_out.Age			= Age;
	vs_out.Life			= Life;
}

#type geometry
#version 450 core

layout(points) in;
layout(points) out;
layout(max_vertices = 254) out;

// Info from the VS
in VS_OUT
{
	flat int		Type;
	flat int		ID;
	vec3			InitPosition;
	vec3			Position;
	vec3			Randomness;
	vec3			Rotation;
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
out vec3			o_Color;
out float			o_Age;
out float			o_Life;

uniform float gDeltaTime;
uniform float gTime;
uniform vec3 gForce;
uniform vec3 gColor;
uniform float gRamndomness;
uniform sampler1D gRandomTexture;
uniform float fParticleLifetime;
uniform uint uiNumMaxParticles;

#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2

uniform mat4 model;

// Get a random number, used for calculate the new direction
vec3 GetRandomDir(float TexCoord)
{
	vec3 Dir = texture(gRandomTexture, TexCoord).xyz;
	Dir -= vec3(0.5, 0.5, 0.5); // Since all tex coordinates goes from 0 to 1, we substract 0.5 so all values will go between -0.5 and 0.5
	return Dir;
}

void main()
{
    float Age = gs_in[0].Age + gDeltaTime;	// Increment the age of the particle
	float AgeZtoO = Age/fParticleLifetime;	// Age of the particle, from 0 to 1 (1=died)
	float fParticleID = float(gs_in[0].ID)/float(uiNumMaxParticles);
	
	if (gs_in[0].Type == PARTICLE_TYPE_EMITTER) {
		// Draw the Emitter
		o_Type = PARTICLE_TYPE_EMITTER;
		o_ID = gs_in[0].ID;
		o_InitPosition = gs_in[0].InitPosition;
		o_Position = gs_in[0].InitPosition;
		o_Randomness = gs_in[0].Randomness;
		o_Rotation = gs_in[0].Rotation;
		o_Color = gColor;
		o_Age = Age;
		o_Life = gs_in[0].Life;
		
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
	else if (gs_in[0].Type == PARTICLE_TYPE_SHELL) {
		// If it's the same particle...
		if (Age<gs_in[0].Life) {
			o_Life = gs_in[0].Life;
			o_Color = gs_in[0].Color;
			o_Age = Age;
			// Put your random formula here
			//o_Position = gs_in[0].Position + AgeZtoO * vec3(0.1,0,0); 
			//o_Position = gs_in[0].InitPosition + (vec3(0,0,0) - gs_in[0].InitPosition) * AgeZtoO;
			o_Position = gs_in[0].InitPosition + (vec3(2,0,0) - gs_in[0].InitPosition) * AgeZtoO + gs_in[0].Randomness; // Put your random formula here
			
			o_Randomness = gs_in[0].Randomness;
		}
		// If particle has died, then we need to restart it
		else {
			o_Life = fParticleLifetime;
			o_Color = gColor;
			o_Age = 0;
			o_Position = gs_in[0].InitPosition;
			o_Randomness = gRamndomness*GetRandomDir(gTime+fParticleID);
		}
		
		
		// Draw the Shell
		o_Type = PARTICLE_TYPE_SHELL;
		o_ID = gs_in[0].ID;
		o_InitPosition = gs_in[0].InitPosition;
		o_Rotation = gs_in[0].Rotation;
		EmitVertex();
		EndPrimitive();		// Generate the particle
		
    }
}


#type fragment
#version 450 core

void main()
{
}