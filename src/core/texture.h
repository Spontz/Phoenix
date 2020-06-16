// texture.h
// Spontz Demogroup

#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include <glad/glad.h>

// flags for texture properties
#define NO_MIPMAP 1
#define CLAMP_TO_EDGE 2
#define CLAMP 4
#define MODULATE 8
#define TEXTURE_1D 16

class Texture {
public:
	std::string	filename;
	std::string type;					// texture_diffuse / texture_specular / texture_normal / texture_height
	int			width, height, components;
	float		mem;						// Size in Mb
	bool		use_linear;
	GLuint		m_textureID;

	Texture();
	virtual ~Texture();

	bool load(const std::string & file_name, bool flip);
	void bind(int TexUnit = 0) const;
	void active(int TexUnit = 0) const;
			
private:

};

#endif