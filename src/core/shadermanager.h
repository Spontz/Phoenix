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
	std::vector<Shader*>	shader;		// Shader list
	bool					forceLoad;	// Force shader loading each time we add a shader (should be enabled when working on slave mode) 

	ShaderManager();
	virtual ~ShaderManager();

	int addShader(string path_vert, string path_frag, string path_geom = "", vector<string> feedbackVaryings = {});
	void unbindShaders();

private:

};

#endif