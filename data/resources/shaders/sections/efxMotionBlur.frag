#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D velocity;

// This is to adapt the effect to any framerate
// uVelocityScale = currentFps / targetFps;
uniform float uVelocityScale;

#define MAX_SAMPLES 32

void main()
{             
	vec2 texelSize = 1.0 / textureSize(scene, 0);
	
	vec2 velocity = texture(velocity, TexCoords).rg;
	velocity *= uVelocityScale;
	
	float speed = length(velocity / texelSize);
	int nSamples = clamp(int(speed), 1, MAX_SAMPLES);
	
	// Blur the texture
	FragColor = texture(scene, TexCoords);
	for (int i = 1; i < nSamples; ++i) {
		vec2 offset = velocity * (float(i) / float(nSamples - 1) - 0.5);
		FragColor += texture(scene, TexCoords + offset);
	}
	FragColor /= float(nSamples);
}
