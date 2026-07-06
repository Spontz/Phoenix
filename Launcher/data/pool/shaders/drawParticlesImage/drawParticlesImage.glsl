#type vertex
#version 440 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 Color;

uniform float gTime;
uniform mat4 gModel;
uniform float gNumParticles;

out int ID0;
out vec4 Color0;

#define PI 3.1415926535897932384626433832795

void main(void)
{
	ID0 = gl_VertexID; // Send the particle ID to Geometry shader
	
	float zero_to_one = gl_VertexID / gNumParticles;
	float sphere = 2.0* PI * zero_to_one;
	
	// Calculate the color of the particle
	Color0 = Color;//vec3(zero_to_one, 1.0-zero_to_one, 0.0);

	// Calculate the new position of the particle
	vec3 new_position = Position;


	gl_Position = gModel * vec4(new_position, 1.0);
}


#type geometry
#version 440 core

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 gVP;
uniform vec3 gCameraPos;
uniform float fParticleSize;
uniform float fParticlesDrawn;

// Info from the VS
in vec4 Color0[];
in int ID0[];

// Info sent to FS: Color and Texture Coords
out vec4 Color1;
out vec2 TexCoord;

void main()
{
	Color1 = Color0[0];

	// Only draw particles if are not alpha discarded
	if (ID0[0]<fParticlesDrawn && Color1.a>0.01) {
	
		vec3 Pos = gl_in[0].gl_Position.xyz;
		vec3 toCamera = normalize(gCameraPos - Pos);
		vec3 up = vec3(0.0, 1.0, 0.0);
		vec3 right = cross(toCamera, up) * fParticleSize;
	
		Pos -= right;
		gl_Position = gVP * vec4(Pos, 1.0);
		TexCoord = vec2(0.0, 0.0);
		EmitVertex();

		Pos.y += fParticleSize;
		gl_Position = gVP * vec4(Pos, 1.0);
		TexCoord = vec2(0.0, 1.0);
		EmitVertex();

		Pos.y -= fParticleSize;
		Pos += right;
		gl_Position = gVP * vec4(Pos, 1.0);
		TexCoord = vec2(1.0, 0.0);
		EmitVertex();

		Pos.y += fParticleSize;
		gl_Position = gVP * vec4(Pos, 1.0);
		TexCoord = vec2(1.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}

#type fragment
#version 440 core
layout (location = 0) out vec4 FragColor;

uniform sampler2D partTexture;

in vec4 Color1;
in vec2 TexCoord;


void main(void)
{
	FragColor = texture(partTexture, TexCoord) * Color1;
}
