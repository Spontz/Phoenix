#type vertex
#version 450 core

layout (location = 0) in int Type;
layout (location = 1) in vec3 Position;
layout (location = 2) in vec3 Velocity;
layout (location = 3) in vec3 Color;
layout (location = 4) in float Age;

out VS_OUT
{
	flat int		Type;
	vec3			Position;
	vec3			Velocity;
	vec3			Color;
	float			Age;
} vs_out;

void main()
{
	vs_out.Type = Type;
	vs_out.Position = Position;
	vs_out.Velocity = Velocity;
	vs_out.Color = Color;
	vs_out.Age = Age;
}

#type geometry
#version 450 core

layout(points) in;
layout(points) out;
layout(max_vertices = 254) out;

// Info from the VS
in VS_OUT
{
	flat int	Type;
	vec3		Position;
	vec3		Velocity;
	vec3		Color;
	float		Age;
} gs_in[];

// Info sent to FS
flat out int	o_Type;
out vec3		o_Position;
out vec3		o_Velocity;
out vec3		o_Color;
out float		o_Age;

#define PARTICLE_TYPE_EMITTER 1
#define PARTICLE_TYPE_SHELL 2

uniform mat4 u_m4Model;
uniform float u_fDeltaTime;
uniform float u_fTime;
uniform vec3 u_v3Force;
uniform vec3 u_v3Color;
uniform sampler1D u_iRandomTexture;
uniform float u_fEmissionTime;
uniform float u_fParticleLifetime;
uniform float u_fRamndomness;

// Get a random number, used for calculate the new direction
vec3 GetRandomDir(float TexCoord)
{
	vec3 Dir = texture(u_iRandomTexture, TexCoord).xyz;
	Dir -= vec3(0.5, 0.5, 0.5); // Since all tex coordinates goes from 0 to 1, we substract 0.5 so all values will go betweel -0.5 and 0.5
	return Dir;
}

void main()
{
    float Age = gs_in[0].Age + u_fDeltaTime; // Increment the age of the particle
	vec3 DeltaP = gs_in[0].Velocity * u_fDeltaTime; // Position Delta: xDelta = v*t
	
	if (gs_in[0].Type == PARTICLE_TYPE_EMITTER) {
		// If it's time to create a new particle shell...
		if (Age >= u_fEmissionTime) {
			o_Type = PARTICLE_TYPE_SHELL;
			o_Position = vec3(u_m4Model*vec4(gs_in[0].Position + DeltaP, 1.0));
			//o_Velocity = gs_in[0].Velocity + u_fRamndomness*GetRandomDir(gs_in[0].Color.x);
			o_Velocity = gs_in[0].Velocity + u_fRamndomness*GetRandomDir(u_v3Color.x);
			o_Color = u_v3Color;				//Apply the global color
			//o_Color = gs_in[0].Color;			//Apply the same color as the emitter
			o_Age = 0.0;
			
			EmitVertex();
			EndPrimitive();	// Generate a new particle from the launcher position
			Age = 0.0;		// Set the age of the emitter to 0, so it can generate new particles later
		}
		// Draw the Emitter
		o_Type = PARTICLE_TYPE_EMITTER;
		o_Position = gs_in[0].Position;
		o_Velocity = gs_in[0].Velocity;
		o_Color = u_v3Color; // Apply the global Color
		o_Age = Age;
		
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }

	vec3 DeltaV = u_v3Force * u_fDeltaTime; // vDelta = accel*tDetla

	// If its a normal particle...
	if (gs_in[0].Type == PARTICLE_TYPE_SHELL) {

		// If the is still alive, we update the values...
		if (Age < u_fParticleLifetime) {

			o_Type = PARTICLE_TYPE_SHELL;
			o_Position = gs_in[0].Position + DeltaP; // x = x0 + xDelta
			o_Velocity = gs_in[0].Velocity + DeltaV; // v = v0 + vDelta
			o_Color = gs_in[0].Color - vec3(1.0)*(u_fDeltaTime/u_fParticleLifetime);
			o_Age = Age;
			
			EmitVertex();
			EndPrimitive(); // Update the particle status and position
		}
	}

}


#type fragment
#version 450 core

void main()
{
}