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
		ModelInstance(SP_Model spModel, uint32_t amount);
		~ModelInstance();

	public:
		SP_Model		m_pModel;				// Model to draw
		uint32_t		m_amount;				// Amount of instances to draw
		glm::mat4*		m_pModelMatrix;		// Model matrice of each instance
		glm::mat4*		m_pPrevModelMatrix;	// Previous model matrice of each instance (useful for effects like motion blur)
		unsigned int	m_matricesBuffer;

		void updateMatrices();

		void drawInstanced(GLuint shaderID, uint32_t startTexUnit = 0);
		void copyMatrices(int instance);
	};

}