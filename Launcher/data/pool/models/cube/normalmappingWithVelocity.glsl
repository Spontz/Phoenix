#type vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

smooth out vec4 vPosition;
smooth out vec4 vPrevPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 prev_projection;
uniform mat4 prev_view;
uniform mat4 prev_model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;
        
    vPosition = projection * view * model * vec4(aPos, 1.0);
	vPrevPosition = prev_projection * prev_view * prev_model * vec4(aPos, 1.0);
	
    gl_Position = vPosition;
}

#type fragment
#version 330 core
//////////////
// Note: This shader works only for 1 light... by now :D
//////////////
// This shader needs 2 color attachments
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 VelocityColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

smooth in vec4 vPosition;
smooth in vec4 vPrevPosition;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_height1;

uniform vec3 lightPos;
uniform vec3 lightColDiff;
uniform vec3 viewPos;

uniform float ambientObjectStrenght; // 0.1 and up, recommended 0.9

uniform float diffuseLightStrenght; // 0.1 and up, recommended 0.9
uniform float specularLightStrenght; // 0.2 and up, recommended 0.8

void main()
{           
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(texture_height1, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
    // get diffuse color
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = ambientObjectStrenght * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = (diffuseLightStrenght*lightColDiff) * diff * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(specularLightStrenght) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
	
		// Calculate the velocity vector
	vec2 a = (vPosition.xy / vPosition.w) * 0.5 + 0.5;
	vec2 b = (vPrevPosition.xy / vPrevPosition.w) * 0.5 + 0.5;
	VelocityColor = vec4(a.x - b.x, a.y - b.y, 0, 1);
}