// Mesh.cpp
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
	assert(0);
}

Mesh::Mesh(std::string nodeName, const aiMesh *pMesh, std::vector<Vertex> vertices, std::vector<unsigned int> indices, const aiMaterial *pMaterial, std::string directory, std::string filename)
	: 
	meshTransform(glm::mat4(1.0)),
	m_nodeName(nodeName),
	m_pMesh(pMesh),
	m_vertices(vertices),
	m_indices(indices),
	m_VertexArray(nullptr)
{
	loadUniqueVerticesPos();

	// Setup the material of our mesh (each mesh has only one material)
	this->m_material.Load(pMaterial, directory, filename);

	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	setupMesh();
}



void Mesh::loadUniqueVerticesPos()
{
	// Loads the unique vertices list
	bool vertexFound = false;
	for (int i = 0; i < m_vertices.size(); i++) {
		vertexFound = false;
		for (int j = 0; j < unique_vertices_pos.size(); j++) {
			if (m_vertices[i].Position == unique_vertices_pos[j]) {
				vertexFound = true;
			}
		}
		if (vertexFound == false) {
			unique_vertices_pos.push_back(m_vertices[i].Position);
			float r = glm::sqrt(	m_vertices[i].Position.x * m_vertices[i].Position.x +
									m_vertices[i].Position.y * m_vertices[i].Position.y +
									m_vertices[i].Position.z * m_vertices[i].Position.z);
			float a = glm::atan(m_vertices[i].Position.y , m_vertices[i].Position.x);
			float b = glm::acos(m_vertices[i].Position.z / r);
			unique_vertices_polar.push_back(glm::vec3(a,b,r));
			//unique_vertices_polar.push_back(glm::polar(m_vertices[i].Position));
		}
	}

}

// initializes all the buffer objects/arrays
void Mesh::setupMesh()
{
	// Allocate Vertex Array
	m_VertexArray = new VertexArray();

	// Create & Load the Vertex Buffer
	VertexBuffer *vertexBuffer = new VertexBuffer(&m_vertices[0], static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex)));
	vertexBuffer->SetLayout({
		{ ShaderDataType::Float3,	"aPos"},
		{ ShaderDataType::Float3,	"aNormal"},
		{ ShaderDataType::Float2,	"aTexCoords"},
		{ ShaderDataType::Float3,	"aTangent"},
		{ ShaderDataType::Float3,	"aBiTangent"},
		{ ShaderDataType::UInt4,	"aBoneID"},
		{ ShaderDataType::Float4,	"aBoneWeight"}
		});

	m_VertexArray->AddVertexBuffer(vertexBuffer);

	// Create & Load the Index Buffer
	IndexBuffer* indexBuffer = new IndexBuffer(&m_indices[0], static_cast<uint32_t>(m_indices.size()));
	m_VertexArray->SetIndexBuffer(indexBuffer);

	m_VertexArray->Unbind();
}


void Mesh::DrawMaterials(GLuint shaderID)
{
	// Send material properties
	glUniform3fv(glGetUniformLocation(shaderID, "color_ambient"), 1, &m_material.colAmbient[0]);
	glUniform3fv(glGetUniformLocation(shaderID, "color_specular"), 1, &m_material.colSpecular[0]);
	glUniform1f(glGetUniformLocation(shaderID, "strenght_specular"), m_material.strenghtSpecular);
	glUniform3fv(glGetUniformLocation(shaderID, "color_diffuse"), 1, &m_material.colDiffuse[0]);

	// Send textures
	unsigned int numTextures = static_cast<unsigned int>(m_material.textures.size());
	for (unsigned int i = 0; i < numTextures; i++)
	{
		if (!(m_material.textures[i].tex)) // Avoid illegal access
			return;
		// TODO: Remove this c_str() command could improve performance?
		// TODO: Deberíamos cargar la ultima texture unit que se ha usado, y emepzar desde allí, en vez de empezar desde 0, ya que puede
		// que se haya usado alguna texture unit anteriormente (por ejemplo, para shadowmaps)
		glUniform1i(glGetUniformLocation(shaderID, m_material.textures[i].shaderName.c_str()), i);
		// and finally bind the texture
		m_material.textures[i].tex->bind(i);
	}
}

// render the mesh
void Mesh::Draw(GLuint shaderID)
{
	// Setup materials for drawing
	DrawMaterials(shaderID);

	// draw mesh
	m_VertexArray->Bind();
	glDrawElements(GL_TRIANGLES, (int)m_indices.size(), GL_UNSIGNED_INT, NULL);
	m_VertexArray->Unbind();

	// always good practice to set everything back to defaults once configured.
	//glBindTextureUnit(0, 0); --> TODO: This gives error on some graphics card (https://community.intel.com/t5/Graphics/intel-uhd-graphics-630-with-latest-driver-will-cause-error-when/td-p/1161376)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}




