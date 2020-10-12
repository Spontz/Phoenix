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
	glm::mat4*		modelMatrix;	// Model matrice of each instance
	glm::mat4*		prev_modelMatrix;	// Previous model matrice of each instance (useful for effects like motion blur)
	unsigned int	matricesBuffer;

	ModelInstance(Model* model, unsigned int amount);
	virtual ~ModelInstance();

	void DrawInstanced(GLuint shaderID);
	void copyMatrices(int instance);
	
private:

};

