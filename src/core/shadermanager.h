// shadermanager.h
// Spontz Demogroup

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H
#include <string>
#include <vector>
#include "core/shader.h"

class ShaderManager {
public:
	std::vector<Shader*>	shader;		// Shader list
	bool					forceLoad;	// Force shader loading each time we add a shader (should be enabled when working on slave mode) 

	ShaderManager();
	virtual ~ShaderManager();

	Shader* addShader(std::string filepath, std::vector<std::string> feedbackVaryings = {});
	void unbindShaders();

private:

};

#endif