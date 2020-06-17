// cubemap.h
// Spontz Demogroup

#ifndef CUBEMAP_H
#define CUBEMAP_H
#include <string>
#include <glad/glad.h>

class Cubemap {
public:
	std::vector<std::string> filename;
	std::vector<int> width;
	std::vector<int> height;
	float mem;
	GLuint cubemapID;

	Cubemap();
	virtual ~Cubemap();

	bool load(std::vector<std::string> faces, bool flip);
	void bind(GLuint TexUnit = 0) const;

private:

};

#endif