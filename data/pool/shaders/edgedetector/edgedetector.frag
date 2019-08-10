#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float intensity;
uniform sampler2D screenTexture;

void main()
{
	float kernel[9];
	vec2 size = textureSize(screenTexture,0);
	float step_w = 1.0/size.x;
	float step_h = 1.0/size.y;
	vec2 offset[9];
	
	int i = 0;
	vec4 sum = vec4(0.0);

	offset[0] = vec2(-step_w, -step_h);
	offset[1] = vec2(0.0, -step_h);
	offset[2] = vec2(step_w, -step_h);

	offset[3] = vec2(-step_w, 0.0);
	offset[4] = vec2(0.0, 0.0);
	offset[5] = vec2(step_w, 0.0);

	offset[6] = vec2(-step_w, step_h);
	offset[7] = vec2(0.0, step_h);
	offset[8] = vec2(step_w, step_h);

	kernel[0] = 0.0; 			kernel[1] = 1.0*intensity;	kernel[2] = 0.0;
	kernel[3] = 1.0*intensity;	kernel[4] = -4.0*intensity;	kernel[5] = 1.0*intensity;
	kernel[6] = 0.0;   			kernel[7] = 1.0*intensity;	kernel[8] = 0.0;

	for( i=0; i<9; i++ )
	{
		vec4 tmp = texture(screenTexture, TexCoords.xy + offset[i]);
		sum += tmp *kernel[i];
	}
	FragColor = sum;
}