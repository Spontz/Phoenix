#version 330 core
// This shader needs 2 color attachments
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 VelocityColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

smooth in vec4 vPosition;
smooth in vec4 vPrevPosition;

struct Light {
    vec3 Position;
    vec3 Color;
};

#define LIGHTS 2
uniform Light light[LIGHTS];
uniform sampler2D texture_diffuse1;

uniform float ambientObjectStrenght; // Recommended 0.01

void main()
{           
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    // ambient
    vec3 ambient = ambientObjectStrenght * color;
    // lighting
    vec3 lighting = vec3(0.0);
    for(int i = 0; i < LIGHTS; i++)
    {
        // diffuse
        vec3 lightDir = normalize(light[i].Position - fs_in.FragPos);
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = light[i].Color * diff * color;      
        vec3 result = diffuse;        
        // attenuation (use quadratic as we have gamma correction)
        float distance = length(fs_in.FragPos - light[i].Position);
        result *= 1.0 / (distance * distance);
        lighting += result;
                
    }
    FragColor = vec4(ambient + lighting, 1.0);

	// Calculate the velocity vector
	vec2 a = (vPosition.xy / vPosition.w) * 0.5 + 0.5;
	vec2 b = (vPrevPosition.xy / vPrevPosition.w) * 0.5 + 0.5;
	VelocityColor = vec4(a.x - b.x, a.y - b.y, 0, 1);
}