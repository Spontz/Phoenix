// texture.h
// Spontz Demogroup

#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include <glad/glad.h>
using namespace std;

// flags for texture properties
#define NO_MIPMAP 1
#define CLAMP_TO_EDGE 2
#define CLAMP 4
#define MODULATE 8
#define TEXTURE_1D 16

class Texture {
public:
	string filename;
	int width, height, components;
	bool use_linear;

	Texture();
	virtual ~Texture();

	bool load(const std::string & file_name);
	void bind(int index = 0) const;
			
private:
	GLuint to_id;
};

#endif