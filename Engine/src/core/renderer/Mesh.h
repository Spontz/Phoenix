// Mesh.h
// Spontz Demogroup


#pragma once

#include "main.h"
#include <glm/glm.hpp>
#include "core/Demokernel.h"
#include "core/renderer/Material.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Shader.h"
#include "core/renderer/VertexArray.h"

#include <string>
#include <vector>

namespace Phoenix {

constexpr uint32_t NUM_BONES_PER_VERTEX = 4; // Number of Bones per Vertex

	struct BoneInfo
	{
		int32_t		id = -1;
		glm::mat4	BoneOffset = glm::mat4(0.0f);
		glm::mat4	FinalTransformation = glm::mat4(0.0f);
	};

	struct VertexBoneData
	{
		GLuint IDs[NUM_BONES_PER_VERTEX];
		GLfloat Weights[NUM_BONES_PER_VERTEX];

		VertexBoneData() { Reset(); }

		void Reset()
		{
			for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
			{
				IDs[i] = 0;
				Weights[i] = 0;
			}
		}

		void AddBoneData(unsigned int BoneID, float Weight);
	};

	struct Vertex {
		glm::vec3		Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3		Normal = { 0.0f, 0.0f, 0.0f };
		glm::vec2		TexCoords = { 0.0f, 0.0f };
		glm::vec3		Tangent = { 0.0f, 0.0f, 0.0f };
		glm::vec3		Bitangent = { 0.0f, 0.0f, 0.0f };
		VertexBoneData	Bone;
	};

	struct UniqueVertex {
		glm::vec3		Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3		PositionPolar = { 0.0f, 0.0f, 0.0f }; // Polar coordinates of the vertex
		glm::vec3		Normal = { 0.0f, 0.0f, 0.0f };
	};

	class Mesh;
	using SP_Mesh = std::shared_ptr<Mesh>;
	using WP_Mesh = std::weak_ptr<Mesh>;

	class Mesh final {
		friend class ModelInstance;	// We allod ModelInstance class to access private memebers

	public:

		Mesh(
			const aiScene* pScene,
			std::string nodeName,
			const aiMesh* pMesh,
			std::vector<Vertex> vertices,
			std::vector<unsigned int> indices,
			const aiMaterial* pMaterial,
			std::string directory,
			std::string filename
		);

	public:
		// render the mesh
		void Draw(GLuint shaderID, uint32_t startTexUnit = 0);

		// Loads the unique vertices positions
		void loadUniqueVerticesPos();

		// Get Material
		Material* getMaterial();

	private:
		// initializes all the buffer objects/arrays
		void setupMesh();

		// Setup materials for drawing
		void setMaterialShaderVars(GLuint shaderID, uint32_t startTexUnit = 0);

	private:
		const aiMesh*				m_pMesh;		// ASSIMP mesh object
		std::vector<unsigned int>	m_indices;		// Indices
		std::vector<Vertex>			m_vertices;		// Vertices
		Material					m_material;		// Material
		SP_VertexArray				m_VertexArray;	// Vertex Array Object

	public:
		std::vector<UniqueVertex>	m_uniqueVertices;	// Unique vertices
		glm::mat4					m_matModel;				// Model Matrix for positioning the mesh
		glm::mat4					m_matPrevModel;			// Previous model Matrix for positioning the mesh (useful for effects like motion blur)
		std::string					m_nodeName;
		uint32_t					m_numVertices;
		uint32_t					m_numFaces;
	};
}
