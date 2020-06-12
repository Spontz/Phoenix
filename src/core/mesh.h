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
	std::vector<Vertex>			vertices;
	std::vector<unsigned int>	indices;
	std::vector<glm::vec3>		unique_vertices_pos; // Unique vertices positions
	Material					material;
	GLuint						VAO;
	std::string					nodeName;
	glm::mat4					meshTransform; // Transformation Matrix for positioning the mesh
	

	Mesh(std::string nodeName, const aiMesh *pMesh, std::vector<Vertex> vertices, std::vector<unsigned int> indices, const aiMaterial *pMaterial, std::string directory, std::string filename);
	// render the mesh
	void Draw(GLuint shaderID);
	
private:
	unsigned int		VBO, EBO;
	const aiMesh		*m_pMesh;

	// initializes all the buffer objects/arrays
	void setupMesh();

	// Loads the unique vertices positions
	void loadUniqueVerticesPos();
};
#endif