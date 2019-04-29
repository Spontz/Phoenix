// text.h
// Spontz Demogroup

#ifndef TEXT_H
#define TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
	GLuint		TextureID;  // ID handle of the glyph texture
	glm::ivec2	Size;       // Size of glyph
	glm::ivec2	Bearing;    // Offset from baseline to left/top of glyph
	GLuint		Advance;    // Offset to advance to next glyph
};

class Text {
public:
	FT_Library ft;
	std::map<GLchar, Character> Characters;

	Text(string vshader_path, string fshader_path, string font_path);
	virtual ~Text();

	void RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
	void glPrintf(float x, float y, const char *message, ...);

private:
	GLuint VAO, VBO;	// Buffer objects for drawing the text
	int shdr_font;		// Shader used for drawing the text
};

#endif