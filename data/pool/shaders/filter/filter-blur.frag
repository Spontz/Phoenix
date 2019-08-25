#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

float kernel[9];

uniform sampler2D screenTexture;
uniform float steps;

vec2 offset[9];

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture2D(image, uv) * 0.1964825501511404;
  color += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}

void main(void)
{
	vec2 step = steps/textureSize(screenTexture, 0);

	int i = 0;
	vec4 sum = vec4(0.0);

	offset[0] = vec2(-step.x, -step.y);
	offset[1] = vec2(0.0, -step.y);
	offset[2] = vec2(step.x, -step.y);

	offset[3] = vec2(-step.x, 0.0);
	offset[4] = vec2(0.0, 0.0);
	offset[5] = vec2(step.x, 0.0);

	offset[6] = vec2(-step.x, step.y);
	offset[7] = vec2(0.0, step.y);
	offset[8] = vec2(step.x, step.y);

	kernel[0] = 1.0/16.0;	kernel[1] = 2.0/16.0;	kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;	kernel[4] = 4.0/16.0;	kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;	kernel[7] = 2.0/16.0;	kernel[8] = 1.0/16.0;


	for( i=0; i<9; i++ )
	{
		vec4 tmp = texture(screenTexture, TexCoords.st + offset[i]);
		sum += tmp * kernel[i];
	}

	FragColor = blur13(screenTexture, TexCoords.st, textureSize(screenTexture, 0).xy, vec2(1,1));
	//FragColor = sum;
}