// Model.h
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

class Model
{
public:
	std::vector<Mesh>	meshes;
	std::string			directory;			// Path of the model file
	std::string			filename;			// Name of the model file
	std::string			filepath;			// Full path of the model file
	bool				playAnimation;		// Do we want to compute the transofrmations for playing animations?
	bool				useCamera;			// Do we want to use the camera of the model?

	// Matrices
	glm::mat4			m_matProjection;	// Projection matrix
	glm::mat4			m_matView;			// View matrix
	glm::mat4			m_matBaseModel;		// Base model matrix (the intial matrix of all sub-meshes)
	glm::mat4			m_matMVP;			// Model-View-Projection matrix
	
	std::vector<Camera*>	m_camera;

	// Stats
	uint32_t			m_statNumVertices;
	uint32_t			m_statNumMeshes;
	uint32_t			m_statNumCameras;
	uint32_t			m_statNumAnimations;

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
	glm::mat4			m_matGlobalInverseTransform; // Global transformation matrix for nodes (vertices relative to bones)
	// Bones info
	std::map<std::string, unsigned int> m_BoneMapping;	// maps a bone name to its index
	std::vector<BoneInfo>	m_BoneInfo;
	uint32_t				m_numBones;
	unsigned int			m_currentCamera;			// Current Camera
	unsigned int			m_currentAnimation;			// Current Animation
	double					m_animDuration;				// Animation duration in seconds

	// Get Stats form the model
	void getStats();

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
};
