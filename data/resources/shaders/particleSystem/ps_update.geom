#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

// Info from the VS
in vec3 Position0[];
in vec3 Velocity0[];
in vec3 Color0[];
in float Age0[];
in float Size0[];
in int Type0[];

// Info sent to FS
out vec3 Position1;
out vec3 Velocity1;
out vec3 Color1;
out float Age1;
out float Size1;
out int Type1;

//uniform float gDeltaTimeMillis;
uniform float gDeltaTime;
uniform float gTime;
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
	Size1 = Size0[0];
	
	// If its an emitter...
	if (Type0[0] == PARTICLE_TYPE_EMITTER) {
		if (Age >= fEmissionTime) {	// If it's time to create a new particle...
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = Position0[0];
			Velocity1 = Velocity0[0];
			Color1 = Color0[0];
			Age1 = 0.0;
			EmitVertex();
			EndPrimitive();	// Generate a new particle from the launcher position
			Age = 0.0;
		}
		Type1 = PARTICLE_TYPE_EMITTER;
		Position1 = Position0[0];
		Velocity1 = Velocity0[0];
		Color1 = Color0[0];
		Age1 = Age;
		EmitVertex();
		EndPrimitive();		// Generate the emitter
    }
	
	// If its a normal particle...
	else {
		vec3 DeltaP = gDeltaTime * Velocity0[0]; // xDelta = v*t
		vec3 DeltaV = vec3(0.0, 0, -10.981) * gDeltaTime; // vDelta = accel*tDetla
		
		// If the is still alive, we update the values...
		if (Age < fParticleLifetime) {
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = Position0[0] + DeltaP; // x = x0 + xDelta
			float randomNum = (Position0[0].x + Position0[0].y + Position0[0].z); 
			//vec3 Dir = GetRandomDir(randomNum);
			//vec3 Dir = vec3(1.0, 1.0, 1.0);
			//Velocity1 = Velocity0[0] + DeltaV/Dir; // v = v0 + vDelta
			Velocity1 = Velocity0[0] + DeltaV; // v = v0 + vDelta
			Color1 = Color0[0] - vec3(1.0, 1.0, 1.0)*gDeltaTime/fParticleLifetime;
			//Color1 = Color0[0] - Color0[0]*(gDeltaTime/fParticleLifetime);

			Age1 = Age;

			EmitVertex();
			EndPrimitive(); // Update the particle status and position
		}
	}
}
