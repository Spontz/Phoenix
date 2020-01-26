#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

// Info from the VS
in vec3 Position0[];
in vec3 Velocity0[];
in vec3 Color0[];
in float Age0[];
in int Type0[];

// Info sent to FS
out vec3 Position1;
out vec3 Velocity1;
out vec3 Color1;
out float Age1;
out int Type1;

//uniform float gDeltaTimeMillis;
uniform float gDeltaTime;
uniform float gTime;
uniform vec3 force;
uniform sampler1D gRandomTexture;
uniform float fEmissionTime;
uniform float fParticleLifetime;

#define PARTICLE_TYPE_EMITTER 0
#define PARTICLE_TYPE_SHELL 1

// Get a random number, used for calculate the new direction
vec3 GetRandomDir(float TexCoord)
{
	vec3 Dir = texture(gRandomTexture, TexCoord).xyz;
	Dir -= vec3(0.5, 0.5, 0.5); // Since all tex coordinates goes from 0 to 1, we substract 0.5 so all values will go betweel -0.5 and 0.5
	return Dir;
}

void main()
{
    float Age = Age0[0] + gDeltaTime; // Increment the age of the particle
	vec3 DeltaP = gDeltaTime * Velocity0[0]; // Position Delta: xDelta = v*t

	// Draw the emitter
	if (Type0[0] == PARTICLE_TYPE_EMITTER) {
		// If it's time to create a new particle...
		if (Age >= fEmissionTime) {
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = Position0[0];// + vec3(0.5*sin(gTime), 0,0);
			float randomNum = (Position0[0].x + Position0[0].y + Position0[0].z); 
			Velocity1 = Velocity0[0] + GetRandomDir(gDeltaTime);
			Color1 = Color0[0];
			Age1 = 0.0;
			EmitVertex();
			EndPrimitive();	// Generate a new particle from the launcher position
			Age = 0.0;		// Set the age of the emitter to 0, so it can generate new particles later
		}	
		Type1 = PARTICLE_TYPE_EMITTER;
		Position1 = Position0[0];// + DeltaP;
		Velocity1 = Velocity0[0];
		Color1 = Color0[0];
		Age1 = Age;
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
	
	// If its a normal particle...
	else {
		vec3 DeltaV = force * gDeltaTime; // vDelta = accel*tDetla
		
		// If the is still alive, we update the values...
		if (Age < fParticleLifetime) {
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = Position0[0] + DeltaP;// + DeltaP; // x = x0 + xDelta
			Velocity1 = Velocity0[0] + DeltaV; // v = v0 + vDelta
			Color1 = Color0[0] - vec3(1.0, 1.0, 1.0)*gDeltaTime/fParticleLifetime;

			Age1 = Age;

			EmitVertex();
			EndPrimitive(); // Update the particle status and position
		}
	}
}
