// font.cpp
// Spontz Demogroup

#include <main.h>
#include "font.h"
#define MAX_FONT_WIDTH 20000

Font::Font(int size, std::string font_path, std::string shader_path)
	:
	textureID(0)
{
	// Load the shader
	Shader *myShad;
	glm::mat4 projection;
	shdr_font = DEMO->shaderManager.addShader(shader_path);
	
	if (shdr_font != -1) {
		myShad = DEMO->shaderManager.shader[shdr_font];
		projection = DEMO->camera->getOrthoMatrix_Projection();
		myShad->use();
	}
	else {
		LOG->Error("Could not load Font shader!");
		return;
	}
		

	myShad->setValue("projection", projection);
	LOG->Info(LogLevel::LOW, "Font shader loaded OK");


	if (FT_Init_FreeType(&ft)) {
		LOG->Error("Freetype library could not be initializad");
		return;
	}
	LOG->Info(LogLevel::LOW, "FreeType lib loaded OK");

	FT_Face face;
	if (ft) {
		if (FT_New_Face(ft, font_path.c_str(), 0, &face)) {
			LOG->Error("Freetype library could not load font: %s", font_path.c_str());
			return;
		}
		FT_Set_Pixel_Sizes(face, 0, size);
		LOG->Info(LogLevel::LOW, "Font %s loaded OK", font_path.c_str());
	}

	FT_GlyphSlot glyphSlot = face->glyph;

	// First we need to know the Atlas texture size, and create it
	width = 0;
	height = 0;
	int char_width = 0; // We will add 1 pixed to the with, to avoid interferences between chars
	for (GLubyte c = 32; c < 128; c++) {
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			LOG->Error("Freetype: Failed to load Glyph: %c", c);
			continue;
		}
		
		// Calculate atlas size
		char_width = glyphSlot->bitmap.width + 1;
		width += char_width;
		height = (int)std::fmax(height, glyphSlot->bitmap.rows);
	}
	// Generate the Atlas texture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D,	0, GL_RED,	width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0); // Load an empty texture, we will fill it later with glTexSubImage2D
	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	LOG->Info(LogLevel::LOW, "Font Atlas texture size is %dx%d pixels", width, height);

	// Now we put all the characters into the texture
	int ox = 0;
	int oy = 0;
	char_width = 0;
	for (GLubyte c = 32; c < 128; c++) {
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			LOG->Error("Freetype: Failed to load Glyph");
			continue;
		}
		char_width = glyphSlot->bitmap.width + 1;
		glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, glyphSlot->bitmap.width, glyphSlot->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, glyphSlot->bitmap.buffer);
		//LOG->Info(LogLevel::LOW, "Inserted char %c, texture in %dx%d pixels, size: %dx%d", c, ox, oy, glyphSlot->bitmap.width, glyphSlot->bitmap.rows);
		// Now store character for later use
		Char character = {
			glm::ivec2(glyphSlot->bitmap.width, glyphSlot->bitmap.rows),
			glm::ivec2(glyphSlot->bitmap_left, glyphSlot->bitmap_top),
			glm::fvec2((float)ox / (float)width, (float)glyphSlot->bitmap.rows / (float)height),
			(GLuint)(glyphSlot->advance.x) >> 6 // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		};
		//LOG->Info(LogLevel::LOW, "Inserted char %c, size: %dx%d, offset: %.4f, offsetSize: %.4f", c, character.Size.x, character.Size.y, character.coordOffset.x, (float)(character.Size.x) / (float)width);
		Characters.insert(std::pair<GLchar, Char>(c, character));

		ox += char_width;
	}
	
	// Free memory
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	LOG->Info(LogLevel::LOW, "Freetype Glyph fonts processed OK, and Freetype Lib unloaded OK");

	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Font::~Font()
{

}

void Font::glPrintf(float x, float y, const char *message, ...) {
	va_list argptr;
	char text[256];

	if (message == NULL)
		return;
	
	va_start(argptr, message);
	vsprintf_s(text, message, argptr);
	va_end(argptr);
	
	RenderText(text, x, y, 0.2f, glm::vec3(1, 1, 1));
}

void Font::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	float stretch = 0.7f;
	// Prevent rendering text if shader has not been properly loaded
	if (shdr_font == -1)
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	scale /= 100.0f; // Divide per 100 in order to match our windows scale (from 0 to 1)
	// Activate corresponding render state	
	Shader *myShad;
	myShad = DEMO->shaderManager.shader[shdr_font];
	myShad->use();

	myShad->setValue("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Render texture atlas
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Char ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = (ch.Size.x * scale)*stretch;
		GLfloat h = (ch.Size.y * scale);

		GLfloat u = ch.coordOffset.x;
		GLfloat usize = (float)(ch.Size.x) / (float)width;
		
		GLfloat v = 0;
		GLfloat vsize = ch.coordOffset.y;

		//LOG->Info(LogLevel::LOW, "Font sizes: char: %c - xypos: %.3f,%.3f - wh: %.3f,%.3f uusize: %.3f,%.3f", *c, xpos, ypos, w, h, u, usize);
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     -ypos - h,   u, v },
			{ xpos,     -ypos,       u, vsize },
			{ xpos + w, -ypos,       u+usize, vsize },

			{ xpos,     -ypos - h,   u, v },
			{ xpos + w, -ypos,       u+usize, vsize },
			{ xpos + w, -ypos - h,   u+usize, v }
		};
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += ((ch.Advance) * scale)*stretch;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}


