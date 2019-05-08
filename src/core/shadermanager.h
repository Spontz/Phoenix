// shadermanager.h
// Spontz Demogroup

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H
#include <string>
#include <vector>
#include "core/shader.h"

using namespace std;

class ShaderManager {
public:
	std::vector<Shader*> shader;	// Shader list

	ShaderManager();
	int addShader(string path_vert, string path_frag, string path_geo = "");
	void unbindShaders();

private:

};

#endif