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

	class ModelInstance;
	using SP_ModelInstance = std::shared_ptr<ModelInstance>;
	using WP_ModelInstance = std::weak_ptr<ModelInstance>;

	class ModelInstance
	{
		struct ModelInstanceStats final {
			uint32_t numTotalFaces = 0;
			uint32_t numTotalVertices = 0;
		};

	public:
		ModelInstance(SP_Model spModel, uint32_t amount);
		~ModelInstance();

	public:
		SP_Model						m_pModel;				// Model to draw
		uint32_t						m_amount;				// Amount of instances to draw
		std::unique_ptr<glm::mat4[]>	m_pModelMatrix;			// Model matrice of each instance
		std::unique_ptr<glm::mat4[]>	m_pPrevModelMatrix;		// Previous model matrice of each instance (useful for effects like motion blur)
		ModelInstanceStats				m_stats;				// ModelInstance Statistics
	private:
		uint32_t						m_vBufferMM_ID;			// Vertex buffer ID where the InstanceModelMatrix are stored, all meshes of the model will store the InstanceModelMatrix in the same Id, so only one ID neds to be stored

	public:
		void updateMatrices();
		void drawInstanced(float currentTime, GLuint shaderID, uint32_t startTexUnit = 0);
		void copyMatrices(int instance);
	};

}