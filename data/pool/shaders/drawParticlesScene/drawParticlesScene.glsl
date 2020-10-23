#type vertex
#version 440 core
layout (location = 1) in vec3 Position;

uniform float gTime;
uniform mat4 gModel;
uniform float gNumParticles;

out int ID0;
out vec3 Color0;

#define PI 3.1415926535897932384626433832795

void main(void)
{
	ID0 = gl_VertexID; // Send the particle ID to Geometry shader
	
	float zero_to_one = gl_VertexID / gNumParticles;
	float sphere = 2.0* PI * zero_to_one;
	
	// Calculate the color of the particle
	//Color0 = vec3(1.0, 0.5+0.5*sin(gTime), 1.0);
	Color0 = vec3(1+sin(((sphere-PI)/2.0)+gTime), 0.0, 0.0);//0.5+0.5*sin(gTime), 1.0);

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
in vec3 Color0[];
in int ID0[];

// Info sent to FS: Color and Texture Coords
out vec3 Color1;
out vec2 TexCoord;

void main()
{
	Color1 = Color0[0];

	if (ID0[0]<fParticlesDrawn) {
	
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

in vec3 Color1;
in vec2 TexCoord;


void main(void)
{
	FragColor = texture(partTexture, TexCoord) * vec4(Color1.rgb, 1.0f);
}