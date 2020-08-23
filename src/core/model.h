﻿// model.h
// Spontz Demogroup


#ifndef MODEL_H
#define MODEL_H

#include "main.h"

#include "core/demokernel.h"
#include "core/mesh.h"
#include "core/shader.h"
#include "core/camera.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

class Model
{
public:
	std::vector<Mesh>	meshes;
	std::string			directory;	// Path of the model file
	std::string			filename;	// Name of the model file
	std::string			filepath;	// Full path of the model file
	bool				playAnimation;	// Do we want to compute the transofrmations for playing animations?
	bool				useCamera;		// Do we want to use the camera of the model?
	glm::mat4			modelTransform;	// Model initial matrix
	glm::mat4			prev_view;		// Previous view model matrix, used for effects like motion blur

	std::vector<Camera*>	m_camera;

	Model();
	virtual ~Model();

	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	bool Load(const std::string& path);
	// draws the model, and thus all its meshes
	void Draw(GLuint shaderID, float currentTime);
	void setAnimation(unsigned int a);
	void setCamera(unsigned int c);

private:
	Assimp::Importer	m_Importer;
	const aiScene*		m_pScene;
	glm::mat4			m_GlobalInverseTransform; // Global transformation matrix for nodes (vertices relative to bones)
	unsigned int		m_NumMeshes;
	unsigned int		m_NumCameras;
	unsigned int		m_NumAnimations;
	// Bones info
	std::map<std::string, unsigned int> m_BoneMapping;	// maps a bone name to its index
	unsigned int			m_NumBones;
	std::vector<BoneInfo>	m_BoneInfo;
	unsigned int			m_currentCamera;			// Current Camera
	unsigned int			m_currentAnimation;			// Current Animation
	double					m_animDuration;				// Animation duration in seconds

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(std::string nodeName, aiMesh *mesh, const aiScene *scene);

	// Process the scene cameras
	void processCameras(const aiScene* scene);

	// Set mesh transformations
	void setMeshesModelTransform();

	// Bones Calculations
	void setBoneTransformations(GLuint shaderProgram, float currentTime);
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
	std::vector<Texture*> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};


#endif