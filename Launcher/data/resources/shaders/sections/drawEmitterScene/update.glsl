#type vertex
#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Velocity;
layout (location = 2) in vec3 Color;
layout (location = 3) in float Age;
layout (location = 4) in int Type;

out VS_OUT
{
	vec3	Position;
	vec3	Velocity;
	vec3	Color;
	float	Age;
	int		Type;
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
#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

// Info from the VS
in VS_OUT
{
	vec3	Position;
	vec3	Velocity;
	vec3	Color;
	float	Age;
	int		Type;
} gs_in[];

// Info sent to FS
out vec3 Position1;
out vec3 Velocity1;
out vec3 Color1;
out float Age1;
out int Type1;

//uniform float gDeltaTimeMillis;
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
	int type = int(gs_in[0].Type);

	if (type == PARTICLE_TYPE_EMITTER) {
		// If it's time to create a new particle shell...
		if (Age >= fEmissionTime) {
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = vec3(model*vec4(gs_in[0].Position + DeltaP, 1.0));
			float randomNum = (gs_in[0].Position.x + gs_in[0].Position.y + gs_in[0].Position.z); 
			Velocity1 = gs_in[0].Velocity + GetRandomDir(Age/fEmissionTime);
			Color1 = gColor;				//Apply the global color
			//Color1 = gs_in[0].Color;		//Apply the same color as the emitter
			Age1 = 0.0;
			EmitVertex();
			EndPrimitive();	// Generate a new particle from the launcher position
			Age = 0.0;		// Set the age of the emitter to 0, so it can generate new particles later
		}	
		// Draw the Emitter
		Type1 = PARTICLE_TYPE_EMITTER;
		Position1 = gs_in[0].Position;
		Velocity1 = gs_in[0].Velocity;
		Color1 = gs_in[0].Color;
		Age1 = Age;
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
	
	// If its a normal particle...
	if (type == PARTICLE_TYPE_SHELL) {
		
		// If the is still alive, we update the values...
		if (Age < fParticleLifetime) {
			vec3 DeltaV = gForce * gDeltaTime; // vDelta = accel*tDetla

			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = gs_in[0].Position + DeltaP; // x = x0 + xDelta
			Velocity1 = gs_in[0].Velocity + DeltaV; // v = v0 + vDelta
			Color1 = gs_in[0].Color - vec3(1.0, 1.0, 1.0)*gDeltaTime/fParticleLifetime;

			Age1 = Age;

			EmitVertex();
			EndPrimitive(); // Update the particle status and position
		}
	}
}


#type fragment
#version 330

void main()
{
}