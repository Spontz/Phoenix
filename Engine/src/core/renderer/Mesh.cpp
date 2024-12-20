﻿// Mesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Shader.h"
#include "core/renderer/Mesh.h"
#include "core/renderer/VertexArray.h"


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace Phoenix {

	void VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
	{
		for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i) {
			if (Weights[i] == 0.0)
			{
				IDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		}

		// should never get here - more bones than we have space for
		//assert(0);
	}

	Mesh::Mesh(
		const aiScene* pScene,
		std::string nodeName
		, const aiMesh* pMesh,
		std::vector<Vertex> vertices,
		std::vector<unsigned int> indices,
		const aiMaterial* pMaterial,
		std::string directory,
		std::string filename
	)
		:
		m_matModel(glm::mat4(1.0f)),
		m_matPrevModel(glm::mat4(1.0f)),
		m_nodeName(nodeName),
		m_pMesh(pMesh),
		m_vertices(vertices),
		m_indices(indices),
		m_numVertices(static_cast<uint32_t>(vertices.size())),
		m_numFaces(static_cast<uint32_t>(indices.size()))
	{
		// Setup the material of our mesh (each mesh has only one material)
		m_material.Load(pMaterial, pScene, directory, filename);

		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();
	}



	void Mesh::loadUniqueVerticesPos()
	{
		if (!m_uniqueVertices.empty())
			m_uniqueVertices.clear();

		

		// Loads the unique vertices list
		bool vertexFound = false;
		for (auto& vertex : m_vertices) {
			vertexFound = false;
			for (auto& uniqueVertex : m_uniqueVertices) {
				if (vertex.Position == uniqueVertex.Position) {
					vertexFound = true;
				}
			}
			if (vertexFound == false) {
				//UniqueVertex newUniqueVertex;
				UniqueVertex newUniqueVertex;
				newUniqueVertex.Position = vertex.Position;
				newUniqueVertex.Normal = vertex.Normal;
				newUniqueVertex.PositionPolar = glm::polar(vertex.Position);
				m_uniqueVertices.emplace_back(newUniqueVertex);
			}
		}

	}

	Material* Mesh::getMaterial()
	{
		return &m_material;
	}

	// initializes all the buffer objects/arrays
	void Mesh::setupMesh()
	{
		// Allocate Vertex Array
		m_VertexArray = std::make_shared<VertexArray>();

		// Create & Load the Vertex Buffer
		auto spVB = std::make_shared<VertexBuffer>(&m_vertices[0], static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)));
		spVB->SetLayout({
			{ ShaderDataType::Float3,	"aPos"},
			{ ShaderDataType::Float3,	"aNormal"},
			{ ShaderDataType::Float2,	"aTexCoords"},
			{ ShaderDataType::Float3,	"aTangent"},
			{ ShaderDataType::Float3,	"aBiTangent"},
			{ ShaderDataType::UInt4,	"aBoneID"},
			{ ShaderDataType::Float4,	"aBoneWeight"}
			});

		m_VertexArray->AddVertexBuffer(spVB);

		// Create & Load the Index Buffer
		auto pIB = std::make_shared<IndexBuffer>(&m_indices[0], static_cast<uint32_t>(m_indices.size()));
		m_VertexArray->SetIndexBuffer(pIB);

		m_VertexArray->unbind();
	}

	void Mesh::setMaterialShaderVars(SP_Shader shader, uint32_t startTexUnit)
	{
		// Send material properties
		shader->setValue("Mat_Ka", m_material.colAmbient);
		shader->setValue("Mat_Ks", m_material.colSpecular);
		shader->setValue("Mat_KsStrenght", m_material.strenghtSpecular);
		shader->setValue("Mat_Kd", m_material.colDiffuse);

		// Send textures
		unsigned int numTextures = static_cast<unsigned int>(m_material.textures.size());
		GLint texUnit = startTexUnit;
		for (unsigned int i = 0; i < numTextures; i++)
		{
			if (!(m_material.textures[i].tex))	// Avoid illegal access
				return;
			texUnit = startTexUnit + i;			// TODO: Check that texUnit is not greater than max TexUnits supported (normally, 32)
			shader->setValue(m_material.textures[i].shaderName, texUnit);
			m_material.textures[i].tex->bind(texUnit);
		}
	}

	// render the mesh
	void Mesh::Draw(SP_Shader shader, uint32_t startTexUnit)
	{
		// Setup materials for drawing
		setMaterialShaderVars(shader, startTexUnit);

		// draw mesh
		m_VertexArray->bind();
		glDrawElements(GL_TRIANGLES, (int)m_indices.size(), GL_UNSIGNED_INT, NULL);
		m_VertexArray->unbind();
	}

}