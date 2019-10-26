// mesh.h
// Spontz Demogroup


#ifndef MESH_H
#define MESH_H


#include "main.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/demokernel.h"
#include "core/texture.h"
#include "core/shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;


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
	unsigned int IDs[NUM_BONES_PER_VERTEX];
	float Weights[NUM_BONES_PER_VERTEX];

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
	glm::vec3		Position;
	glm::vec3		Normal;
	glm::vec2		TexCoords;
	glm::vec3		Tangent;
	glm::vec3		Bitangent;
	VertexBoneData	Bone;
};



class Mesh {
public:
	vector<Vertex>			vertices;
	vector<unsigned int>	indices;
	vector<int>				textures;

	GLuint					VAO;

	Mesh(const aiScene *pScene, const aiMesh *pMesh, vector<Vertex> vertices, vector<unsigned int> indices, vector<int> textures, vector<BoneInfo> boneInfo, map<std::string, unsigned int> boneMapping);
	// render the mesh
	void Draw(Shader shader, float currentTime);
	void boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms);
	void setBoneTransformations(GLuint shaderProgram, GLfloat currentTime);

private:
	unsigned int VBO, EBO;
	const aiScene		*m_pScene;
	const aiMesh		*m_pMesh;
	glm::mat4			m_GlobalInverseTransform;	// Global transformation matrix for nodes (vertices relative to bones)
	unsigned int		m_NumBones;					// Number of bones
	double				m_animDuration;				// Animation duration in seconds
	unsigned int		m_currentAnimation;			// Current Animation
	vector<BoneInfo>	m_BoneInfo;
	map<std::string, unsigned int>	m_BoneMapping;	// Stores the map between the bone name to its index

	// initializes all the buffer objects/arrays
	void setupMesh();

	// Bones Transformations
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
};
#endif