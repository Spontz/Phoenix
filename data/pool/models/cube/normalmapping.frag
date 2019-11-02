#version 330 core
//////////////
// Note: This shader works only for 1 light... by now :D
//////////////
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

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
}