// modelinstance.h
// Spontz Demogroup


#pragma once

#include "main.h"

#include "core/demokernel.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "core/camera.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

class ModelInstance
{
public:
	Model*			model;			// Model to draw
	unsigned int	amount;			// Amount of instances to draw
	glm::mat4*		modelMatrices;	// Model matrice of each instance
	unsigned int matricesBuffer;

	ModelInstance(Model* model, unsigned int amount);
	virtual ~ModelInstance();

	void DrawInstanced(GLuint shaderID);
	
private:

};

