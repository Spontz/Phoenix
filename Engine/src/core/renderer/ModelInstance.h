// ModelInstance.h
// Spontz Demogroup


#pragma once

#include "main.h"

#include "core/Demokernel.h"
#include "core/renderer/Mesh.h"
#include "core/renderer/Shader.h"
#include "core/renderer/Camera.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

namespace Phoenix {

	class ModelInstance
	{
	public:
		ModelInstance(SP_Model model, unsigned int amount);
		~ModelInstance();

	public:
		SP_Model		pModel;				// Model to draw
		unsigned int	amount;				// Amount of instances to draw
		glm::mat4*		pModelMatrix;		// Model matrice of each instance
		glm::mat4*		pPrevModelMatrix;	// Previous model matrice of each instance (useful for effects like motion blur)
		unsigned int	matricesBuffer;

		void updateMatrices();

		void drawInstanced(GLuint shaderID, uint32_t startTexUnit = 0);
		void copyMatrices(int instance);
	};

}