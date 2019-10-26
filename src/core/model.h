﻿// model.h
// Spontz Demogroup


#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "core/demokernel.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "main.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class Model
{
public:

	
	vector<Mesh>	meshes;
	string			directory;	// Path of the model file
	string			filename;	// Name of the model file
	string			filepath;	// Full path of the model file
	bool			gammaCorrection;

	// constructor, expects a filepath to a 3D model.
	Model(string const &path, bool gamma = false);
	virtual ~Model();

	// draws the model, and thus all its meshes
	void Draw(Shader shader, float currentTime);
	unsigned int getNumAnimations();
	void setAnimation(unsigned int a);

private:
	Assimp::Importer	m_Importer;
	const aiScene*		m_pScene;
	glm::mat4			m_GlobalInverseTransform; // Global transformation matrix for nodes (vertices relative to bones)
	unsigned int		m_NumMeshes;
	// Bones info
	std::map<std::string, unsigned int> m_BoneMapping; // maps a bone name to its index
	unsigned int			m_NumBones;
	std::vector<BoneInfo>	m_BoneInfo;
	unsigned int			currentAnimation;			// Current Animation
	double					m_animDuration;				// Animation duration in seconds

	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path);
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);

	// Bones Calculations
	void setBoneTransformations(GLuint shaderProgram, GLfloat currentTime);
	void boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms);
	// Bones Transformations
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture array (from texture manager).
	vector<int> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};


#endif