// mesh.cpp
// Spontz Demogroup

#include "main.h"
#include "core/texture.h"
#include "core/shader.h"
#include "core/mesh.h"


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#define POSITION_LOCATION		0
#define NORMAL_LOCATION			1
#define TEX_COORD_LOCATION		2
#define TANGENT_LOCATION		3
#define BITANGENT_LOCATION		4
#define BONE_ID_LOCATION		5
#define BONE_WEIGHT_LOCATION	6

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
{
	this->meshTransform = glm::mat4(1.0); // Load identity matrix by default
	this->nodeName = nodeName;
	this->m_pMesh = pMesh;
	this->m_vertices = vertices;
	this->m_indices = indices;
	
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
	// Create VAO
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Load data into vertex buffers
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

	// Load data into index buffers (element buffer)
	glGenBuffers(1, &m_indicesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(TANGENT_LOCATION);
	glVertexAttribPointer(TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(BITANGENT_LOCATION);
	glVertexAttribPointer(BITANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	// Bone Vertex ID's as Unsigned INT
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)(offsetof(Vertex, Bone) + offsetof(VertexBoneData, IDs)));
	
	// Bone Vertex Weights
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Bone) + offsetof(VertexBoneData, Weights)));

	glBindVertexArray(0);
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
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, (int)m_indices.size(), GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	//glBindTextureUnit(0, 0); --> TODO: This gives error on some graphics card (https://community.intel.com/t5/Graphics/intel-uhd-graphics-630-with-latest-driver-will-cause-error-when/td-p/1161376)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}




