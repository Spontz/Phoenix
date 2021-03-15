#type vertex
#version 330

layout (location = 0) in vec3 Position;


void main()
{
	gl_Position = vec4(Position, 1.0);
}

#type geometry
#version 330

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 gVP;
uniform vec3 gCameraPos;
uniform float gBillboardSize;

out vec2 TexCoord;

void main()
{
	vec3 Pos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(gCameraPos - Pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(toCamera, up) * gBillboardSize;

	Pos -= right;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 0.0);
	EmitVertex();

	Pos.y += gBillboardSize;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 1.0);
	EmitVertex();

	Pos.y -= gBillboardSize;
	Pos += right;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 0.0);
	EmitVertex();

	Pos.y += gBillboardSize;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}                                                                                   

#type fragment
#version 330

uniform sampler2D gColorMap;

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
	FragColor = texture(gColorMap, TexCoord);

//	if (FragColor.r >= 0.9 && FragColor.g >= 0.9 && FragColor.b >= 0.9) {
//		discard;
//    }
}

