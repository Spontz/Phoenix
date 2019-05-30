// cubemap.h
// Spontz Demogroup

#ifndef CUBEMAP_H
#define CUBEMAP_H
#include <string>
#include <glad/glad.h>
using namespace std;

class Cubemap {
public:
	vector<string> filename;
	vector<int> width;
	vector<int> height;
	GLuint cubemapID;

	Cubemap();
	virtual ~Cubemap();

	bool load(vector<std::string> faces, bool flip);
	void bind() const;
	void active(int index = 0) const;

private:

};

#endif