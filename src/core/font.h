// font.h
// Spontz Demogroup

#ifndef FONT_H
#define FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

struct Char {
	glm::ivec2	Size;       // Size of glyph
	glm::ivec2	Bearing;    // Offset from baseline to left/top of glyph
	glm::fvec2	coordOffset;// Coordinates offset to next glyph
	GLuint		Advance;    // Offset to advance to next glyph
};

class Font {
public:
	FT_Library ft;
	std::map<GLchar, Char> Characters;

	Font(int size, std::string font_path, std::string shader_path);
	virtual ~Font();

	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
	void glPrintf(float x, float y, const char *message, ...);

private:
	GLuint		textureID;		// ID handle of the texture atlas
	GLuint		width, height;	// Texture atlas size
	GLuint		VAO, VBO;		// Buffer objects for drawing the text
	int			shdr_font;		// Shader used for drawing the text
};

#endif