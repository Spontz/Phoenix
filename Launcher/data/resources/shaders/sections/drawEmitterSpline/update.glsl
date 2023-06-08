#type vertex
#version 450 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Velocity;
layout (location = 2) in float StartTime;
layout (location = 3) in vec3 InitVelocity;
layout (location = 4) in vec3 Color;

// Info sent to Transform Feedback Buffer
out vec3	o_Position;
out vec3	o_Velocity;
out float	o_StartTime;
out vec3	o_InitVelocity;
out vec3	o_Color;

uniform mat4 u_m4Model;
uniform float u_fDeltaTime;
uniform float u_fTime;
uniform vec3 u_v3Position;  // Particle emitter position
uniform vec3 u_v3Color;		// Particle emitter color

uniform vec3 u_v3Force;

uniform float u_fEmissionTime;
uniform float u_fParticleLifetime;

void main()
{

	vec4 P = vec4(Position.xyz, 1.0); // Need to be here for the compilation
    gl_Position = P;                    // Need to be here for the compilation

    o_Position = Position;			// Init the output variable
    o_Velocity = Velocity;			// Init the output variable
    o_StartTime = StartTime;		// Init the output variable
    o_InitVelocity = InitVelocity;	// Init the output variable
    o_Color = Color;				// Init the output variable
    if( u_fTime >= StartTime ) {   // Check if the particle is born
        float age = u_fTime - StartTime;   // Get the age of the particle
        if( age > u_fParticleLifetime ) {                 // Check if the particle is dead
            // The particle is past its lifetime, recycle.
            o_Position = u_v3Position;    // Reset position
			//o_Position = vec3(u_m4Model*vec4(u_v3Position, 1.0));
            o_Velocity = InitVelocity;    // Reset velocity
            o_StartTime = u_fTime;        // Reset start time to actual time
			o_Color = u_v3Color;
         } 
         else 
         {
            // The particle is alive, update.
            o_Position += o_Velocity * u_fDeltaTime;   // Scale the translation by the time
			//o_Velocity += u_v3Force * u_fDeltaTime;  // Amplify the velocity
         }
    }else{
        o_Position = u_v3Position;         // Set position (for world space)
		//o_Position = vec3(u_m4Model*vec4(u_v3Position, 1.0));
    }
}

#type fragment
#version 450 core

void main()
{

}