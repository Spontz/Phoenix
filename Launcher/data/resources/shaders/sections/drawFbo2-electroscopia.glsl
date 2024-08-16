#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float minXDraw; // Donde se empieza a dibujar la imagen (de 0 a 1), si se pone al 0.2, se empezará a dibujar la imagen al 20% de la imagen
uniform float maxXDraw; // Donde se termina a dibujar la imagen
uniform float tolerance;
uniform sampler2D screenTexture;

uniform float drawMode;

vec4 drawElectroscopia() {
	vec4 col;

	if ((TexCoords.x >= minXDraw) && (TexCoords.x <= maxXDraw)) {
		return texture(screenTexture, TexCoords); //Inside
	}
	else if (TexCoords.x >= maxXDraw)
		return texture(screenTexture, vec2(maxXDraw, TexCoords.y)); // Right side
	else if (TexCoords.x <= minXDraw)
		return texture(screenTexture, vec2(minXDraw, TexCoords.y)); // Left side
	else
		discard; // should never happen
	}

	vec4 drawGradient (float gradientSize) {
	if ((TexCoords.x > minXDraw) && (TexCoords.x < maxXDraw)) {
	// El fractor gradiente se usa para que el dibujado no sea abrupto, y el dibujado se haga progresivamente
	float gradient = 1.0;
	if ((TexCoords.x > (maxXDraw-gradientSize)) && (TexCoords.x < (maxXDraw+gradientSize)))
		gradient = -TexCoords.x/gradientSize + maxXDraw/gradientSize;

	return gradient * texture(screenTexture, TexCoords);
	}
	else
	  discard;
}

void main()
{
vec4 color;

	if (drawMode > -0.1 && drawMode < 0.1)
		color = drawGradient(tolerance);
	else if (drawMode > 0.9 && drawMode < 1.1)
		color = drawElectroscopia();

	FragColor = color;
}