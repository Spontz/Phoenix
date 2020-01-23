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
