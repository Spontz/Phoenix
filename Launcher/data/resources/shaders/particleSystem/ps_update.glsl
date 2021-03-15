#type vertex
#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Velocity;
layout (location = 2) in vec3 Color;
layout (location = 3) in float Age;
layout (location = 4) in float Size;
layout (location = 5) in int Type;

out vec3 Position0;
out vec3 Velocity0;
out vec3 Color0;
out float Age0;
out float Size0;
out int Type0;

void main()
{
	Position0 = Position;
	Velocity0 = Velocity;
	Color0 = Color;
	Age0 = Age;
	Size0 = Size;
	Type0 = Type;
}

#type geometry
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
uniform float gLauncherLifetime;
uniform float gShellLifetime;

#define PARTICLE_TYPE_LAUNCHER 0
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
	
	Color1 = Color0[0];
	Size1 = Size0[0];

	if (Type0[0] == PARTICLE_TYPE_LAUNCHER) {
		if (Age >= gLauncherLifetime) {
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = Position0[0];
			vec3 Dir = GetRandomDir(gTime);
			//Dir.y = max(Dir.y, 0.5); // We make sure that we emmit up
			//Velocity1 = normalize(Dir);
			Velocity1 = Velocity0[0];//+ normalize(Dir)/1.0; // Divide by random factor
			Age1 = 0.0;
			EmitVertex();
			EndPrimitive();
			Age = 0.0;
		}
		Type1 = PARTICLE_TYPE_LAUNCHER;
		Position1 = Position0[0];
		Velocity1 = Velocity0[0];
		Age1 = Age;
		EmitVertex();
		EndPrimitive();
    }
	// If its an emmitted particle...
	else {
		vec3 DeltaP = gDeltaTime * Velocity0[0]; // xDelta = v*t
		vec3 DeltaV = vec3(0.0, -0.0981, 0.0) * gDeltaTime; // vDelta = accel*tDetla
		DeltaV += vec3(0.1, 0.0, 0.0) * gDeltaTime; // vDelta = accel*tDetla

		//vec3 DeltaP = gDeltaTime * Velocity0[0]; // xDelta = v*t
		//vec3 DeltaV = vec3(gDeltaTime*1000) * (0.0, -9.81, 0.0); // vDelta = accel*tDetla

		if (Age < gShellLifetime) {
			Type1 = PARTICLE_TYPE_SHELL;
			Position1 = Position0[0] + DeltaP; // x = x0 + xDelta
			float randomNum = (Position0[0].x + Position0[0].y + Position0[0].z); 
			vec3 Dir = GetRandomDir(randomNum);
			Velocity1 = Velocity0[0] + DeltaV/Dir; // v = v0 + vDelta
			Age1 = Age;
			EmitVertex();
			EndPrimitive(); // Generate the particle!
		}
	}
}


#type fragment
#version 330

void main()
{
}