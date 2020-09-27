// mesh.h
// Spontz Demogroup


#ifndef MESH_H
#define MESH_H


#include "main.h"

#include <glm/glm.hpp>

#include "core/demokernel.h"
#include "core/material.h"
#include "core/texture.h"
#include "core/shader.h"

#include <string>
#include <vector>

#define NUM_BONES_PER_VERTEX	4 // Number of Bones per Vertex

struct BoneInfo
{
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransformation;

	BoneInfo()
	{
		BoneOffset = glm::mat4(0.0f);
		FinalTransformation = glm::mat4(0.0f);
	}
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

class Mesh {
public:
	std::vector<glm::vec3>		unique_vertices_pos;	// Unique vertices cartesian positions
	std::vector<glm::vec3>		unique_vertices_polar;	// Unique vertices polar position (x=distance, y=alpha, z=beta)
	glm::mat4					meshTransform;			// Transformation Matrix for positioning the mesh
	std::string					nodeName;
	
	Mesh(std::string nodeName, const aiMesh *pMesh, std::vector<Vertex> vertices, std::vector<unsigned int> indices, const aiMaterial *pMaterial, std::string directory, std::string filename);
	// render the mesh
	void Draw(GLuint shaderID);

private:
	friend class ModelInstance;			// We allod ModelInstance class to access private memebers

	GLuint						m_VAO;	// Vertex Array object
	GLuint						m_vertexBuffer;	// Vertex Buffer Object
	GLuint						m_indicesBuffer;	// Element Buffer Object
	const aiMesh				*m_pMesh;	// ASSIMP mesh object
	std::vector<unsigned int>	m_indices;	// Indices
	std::vector<Vertex>			m_vertices;	// Vertices
	Material					m_material;	// Material


	// initializes all the buffer objects/arrays
	void setupMesh();

	// Setup materials for drawing
	void DrawMaterials(GLuint shaderID);

	// Loads the unique vertices positions
	void loadUniqueVerticesPos();
};
#endif