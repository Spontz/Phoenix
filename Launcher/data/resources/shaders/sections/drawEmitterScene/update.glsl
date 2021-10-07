#type vertex
#version 450 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Velocity;
layout (location = 2) in vec3 Color;
layout (location = 3) in float Age;
layout (location = 4) in int Type;

out VS_OUT
{
	vec3			Position;
	vec3			Velocity;
	vec3			Color;
	float			Age;
	flat int		Type;
} vs_out;

void main()
{
	vs_out.Position = Position;
	vs_out.Velocity = Velocity;
	vs_out.Color = Color;
	vs_out.Age = Age;
	vs_out.Type = Type;
}

#type geometry
#version 450 core

layout(points) in;
layout(points) out;
layout(max_vertices = 254) out;

// Info from the VS
in VS_OUT
{
	vec3		Position;
	vec3		Velocity;
	vec3		Color;
	float		Age;
	flat int	Type;
} gs_in[];

// Info sent to FS
out vec3		o_Position;
out vec3		o_Velocity;
out vec3		o_Color;
out float		o_Age;
flat out int	o_Type;

uniform float gDeltaTime;
uniform float gTime;
uniform vec3 gForce;
uniform vec3 gColor;
uniform sampler1D gRandomTexture;
uniform float fEmissionTime;
uniform float fParticleLifetime;

#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2

uniform mat4 model;

// Get a random number, used for calculate the new direction
vec3 GetRandomDir(float TexCoord)
{
	vec3 Dir = texture(gRandomTexture, TexCoord).xyz;
	Dir -= vec3(0.5, 0.5, 0.5); // Since all tex coordinates goes from 0 to 1, we substract 0.5 so all values will go betweel -0.5 and 0.5
	return Dir;
}

void main()
{
    float Age = gs_in[0].Age + gDeltaTime; // Increment the age of the particle
	vec3 DeltaP = gDeltaTime * gs_in[0].Velocity; // Position Delta: xDelta = v*t
	
	if (gs_in[0].Type == PARTICLE_TYPE_EMITTER) {
		// If it's time to create a new particle shell...
		if (Age >= fEmissionTime) {
			o_Type = PARTICLE_TYPE_SHELL;
			o_Position = vec3(model*vec4(gs_in[0].Position + DeltaP, 1.0));
			o_Velocity = gs_in[0].Velocity + GetRandomDir(Age/fEmissionTime);
			o_Color = gColor;				//Apply the global color
			//o_Color = gs_in[0].Color;		//Apply the same color as the emitter
			o_Age = 0.0;
			
			EmitVertex();
			EndPrimitive();	// Generate a new particle from the launcher position
			Age = 0.0;		// Set the age of the emitter to 0, so it can generate new particles later
		}
		// Draw the Emitter
		o_Type = PARTICLE_TYPE_EMITTER;
		o_Position = gs_in[0].Position;
		o_Velocity = gs_in[0].Velocity;
		o_Color = gColor; // Apply the global Color
		o_Age = Age;
		
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
/*
	// If its a normal particle...
	if (gs_in[0].Type == PARTICLE_TYPE_SHELL) {
		
		// If the is still alive, we update the values...
		if (Age < fParticleLifetime) {
			vec3 DeltaV = gForce * gDeltaTime; // vDelta = accel*tDetla

			o_Type = PARTICLE_TYPE_SHELL;
			o_Position = gs_in[0].Position + DeltaP; // x = x0 + xDelta
			o_Velocity = gs_in[0].Velocity + DeltaV; // v = v0 + vDelta
			o_Color = gs_in[0].Color - 0.01*vec3(1.0, 1.0, 1.0)*(Age/fParticleLifetime);
			o_Age = Age;
			
			EmitVertex();
			EndPrimitive(); // Update the particle status and position
		}
	}
*/
}


#type fragment
#version 450 core

void main()
{
}