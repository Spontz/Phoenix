// model.cpp
// Spontz Demogroup

#include "main.h"
#include "core/texture.h"
#include "core/shader.h"
#include "core/mesh.h"
#include "core/model.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;


// For converting between ASSIMP and glm
static inline glm::vec3 vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::vec2 vec2_cast(const aiVector3D &v) { return glm::vec2(v.x, v.y); } // it's aiVector3D because assimp's texture coordinates use that
static inline glm::quat quat_cast(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }
static inline glm::mat4 mat4_cast(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
static inline glm::mat4 mat4_cast(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }

Model::Model(bool gamma)
{
	gammaCorrection = gamma;
	m_NumMeshes = 0; 
	m_NumBones = 0;
	playAnimation = false;			// By default, animations are disabled
	currentAnimation = 0;
	modelTransform = glm::mat4(1.0f);	// Load Identity matrix by default
}

Model::~Model()
{
	meshes.clear();
	directory = "";
	filename = "";
	filepath = "";
}

void Model::Draw(GLuint shaderID, float currentTime)
{
	// Set the Bones transformations and send the Bones info to the Shader (gBones uniform)
	if (this->playAnimation)
		setBoneTransformations(shaderID, currentTime);
	else
		setMeshesModelTransform(this->modelTransform);	// Load the model transformation on all sub-meshes, if no animation is required

	// Then, draw the meshes
	for (unsigned int i = 0; i < meshes.size(); i++) {
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, &(meshes[i].meshTransform[0][0]));
		meshes[i].Draw(shaderID);
	}
}

unsigned int Model::getNumAnimations()
{
	return m_pScene->mNumAnimations;
}

void Model::setAnimation(unsigned int a)
{
	if (a >= 0 && a < getNumAnimations()) {
		currentAnimation = a;
	}
}

// Returns false if model has not been properly loaded
bool Model::Load(string const &path)
{
	filepath = path;
	// read file via ASSIMP
	
	m_pScene = m_Importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	// check for errors
	if (!m_pScene || m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_pScene->mRootNode) // if is Not Zero
	{
		LOG->Error("Error loading file [%s]: %s", filepath.c_str(), m_Importer.GetErrorString());
		return false;
	}
	// retrieve the directory path of the filepath and the filename (without the data folder, because each loader adds the data folder)
	directory = filepath.substr(0, filepath.find_last_of('/'));
	filename = filepath.substr(filepath.find_last_of('/')+1, filepath.length());
	LOG->Info(LOG_LOW, "Loading Model: %s", filename.c_str());

	// Get transformation matrix for nodes (vertices relative to bones)
	m_GlobalInverseTransform = mat4_cast(m_pScene->mRootNode->mTransformation);
	m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
	// process ASSIMP's root node recursively
	processNode(m_pScene->mRootNode, m_pScene);

	// Count total number of meshes
	m_NumMeshes = static_cast<unsigned int>(meshes.size());
	return true;
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode *node, const aiScene *scene)
{
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		LOG->Info(LOG_LOW, "Reading node name: %s", node->mName.data);
		meshes.push_back(processMesh(node->mName.data, mesh, scene));
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}


Mesh Model::processMesh(string nodeName, aiMesh *mesh, const aiScene *scene)
{
	// data to fill
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<int> textures;

	LOG->Info(LOG_LOW, "Loading mesh: %s", mesh->mName.C_Str());

	if (mesh->HasTangentsAndBitangents() == false)
		LOG->Info(LOG_MED, "Warning, the loaded mesh has no Tangents and BiTangents! Normal will be copied there.");

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		if (mesh->HasTangentsAndBitangents()) {
			// tangent
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
			// bitangent
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else {
			vertex.Tangent = glm::vec3(0, 0, 0);
			vertex.Bitangent = glm::vec3(0, 0, 0);
		}
			
		vertices.push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	
	// Process Bones of this mesh and loads it into the "BoneInfo" and "BoneMapping" lists
	// BoneInfo and BoneMapping are like an indexed database of bones (TODO: Put all the Bones stuff in a Class)
	for (unsigned int i = 0; i < mesh->mNumBones; ++i)
	{
		unsigned int BoneIndex = 0;
		string boneName(mesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
		{
			// Allocate an index for the new bone
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);

			// Store the bone mapping: Bone Name + Bone Position in array
			m_BoneInfo[BoneIndex].BoneOffset = mat4_cast(mesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[boneName] = BoneIndex;
		}
		else
		{
			BoneIndex = m_BoneMapping[boneName];
		}

		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
		{
			unsigned int VertexID = mesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[j].mWeight;
			vertices[VertexID].Bone.AddBoneData(BoneIndex, Weight);
		}
	}

	// Process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	// return a mesh object created from the extracted mesh data
	return Mesh(nodeName, mesh, vertices, indices, material, directory, filename);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<int> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
	vector<int> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString filepath;
		string fullpath;
		mat->GetTexture(type, i, &filepath);
		if (0 == strcmp(filepath.C_Str(), "$texture_dummy.bmp"))				// Prevent a bug in assimp: In some cases, the texture by default is named "$texture_dummy.bmp"
			filepath = filename.substr(0, filename.find_last_of('.')) + ".jpg";	// In that case, we change this to "<model_name.jpg>"
		fullpath = directory + "/" + filepath.C_Str();
		int tex = DEMO->textureManager.addTexture(fullpath.c_str(), false, typeName);
		textures.push_back(tex);
	}
	return textures;
}

void Model::setMeshesModelTransform(glm::mat4 &modelMatrix)
{
	for (int i = 0; i < meshes.size(); i++)
		meshes[i].meshTransform = modelMatrix;
}

/////////////// Bones calculations
// TODO: Do a Bones Class, with all this calculations
void Model::setBoneTransformations(GLuint shaderProgram, float currentTime)
{
	// TODO: Hacer que los Transforms sea una propiedad de cada uno del Bone, asi no se tiene q hacer
	// Resize ni cosas raras, y asi ya tenemos un vector de transformaciones de tamaño fijo
	if (m_pScene->HasAnimations()) {
		std::vector<glm::mat4> Transforms;
		boneTransform(currentTime, Transforms);
		if (Transforms.size() > 0)
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "gBones"), (GLsizei)Transforms.size(), GL_FALSE, &Transforms[0][0][0]);
		
	}
}


void Model::boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms)
{
	//TODO: I think that this line does not make any sense... because its overwritten later
	m_animDuration = (float)m_pScene->mAnimations[currentAnimation]->mDuration;

	// Calc animation duration
	unsigned int numPosKeys = m_pScene->mAnimations[currentAnimation]->mChannels[0]->mNumPositionKeys;
	m_animDuration = m_pScene->mAnimations[currentAnimation]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;

	float TicksPerSecond = (float)(m_pScene->mAnimations[currentAnimation]->mTicksPerSecond != 0 ?
									m_pScene->mAnimations[currentAnimation]->mTicksPerSecond : 25.0f);
	float TimeInTicks = timeInSeconds * TicksPerSecond;
	float AnimationTime = (float)fmod(TimeInTicks, m_animDuration);

	ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, this->modelTransform);

	Transforms.resize(m_NumBones);

	for (unsigned int i = 0; i < m_NumBones; i++) {
		Transforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}

void Model::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[currentAnimation];

	glm::mat4 NodeTransformation = mat4_cast(pNode->mTransformation);

	if (AnimationTime > 0)
		int kk = 0;
	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		glm::vec3 scale = glm::vec3(Scaling.x, Scaling.y, Scaling.z);
		glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), scale);

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		glm::quat rotation = quat_cast(RotationQ);
		glm::mat4 RotationM = glm::toMat4(rotation);

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		glm::vec3 translation = glm::vec3(Translation.x, Translation.y, Translation.z);
		glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), translation);
		/*
		// Show debug info
		LOG->Info(LOG_LOW, "Node Anim: %s, second: %.3f", pNodeAnim->mNodeName.data, AnimationTime);
		LOG->Info(LOG_LOW, "S: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);
		LOG->Info(LOG_LOW, "R: %.3f, %.3f, %.3f, %.3f", RotationQ.x, RotationQ.y, RotationQ.z, RotationQ.w);
		LOG->Info(LOG_LOW, "T: %.3f, %.3f, %.3f", translation.x, translation.y, translation.z);
		*/
		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM *ScalingM;
	}

	// Combine with node Transformation with Parent Transformation
	glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;
	
	// TODO: Guarrada, mirar de hacerlo mejor y no usar el size()
	for (int i = 0; i < this->meshes.size(); i++) {
		if (NodeName == this->meshes[i].nodeName) {
			this->meshes[i].meshTransform = m_GlobalInverseTransform * GlobalTransformation;
			/*LOG->Info(LOG_LOW, "Aqui toca guardar la matriz, para el objeto: %s, que es la mesh: %i [time: %.3f]", NodeName.c_str(), i, AnimationTime);
			glm::mat4 M = GlobalTransformation;
			LOG->Info(LOG_LOW, "M: [%.2f, %.2f, %.2f, %.2f], [%.2f, %.2f, %.2f, %.2f], [%.2f, %.2f, %.2f, %.2f], [%.2f, %.2f, %.2f, %.2f]",
				M[0][0], M[0][1], M[0][2], M[0][3],
				M[1][0], M[1][1], M[1][2], M[1][3], 
				M[2][0], M[2][1], M[2][2], M[2][3], 
				M[3][0], M[3][1], M[3][2], M[3][3]);
				*/
		}
	}

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
		unsigned int BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

// Bone position formulas
unsigned int Model::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}
	assert(0);
	return 0;
}

unsigned int Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}
	assert(0);
	return 0;
}

unsigned int Model::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}
	assert(0);
	return 0;
}

void Model::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void Model::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
	/*LOG->Info(LOG_LOW, "rot: factor: %.3f", Factor);
	LOG->Info(LOG_LOW, "rot: Start: %.3f, %.3f, %.3f, %.3f", StartRotationQ.x, StartRotationQ.y, StartRotationQ.z, StartRotationQ.w);
	LOG->Info(LOG_LOW, "rot: End:   %.3f, %.3f, %.3f, %.3f", EndRotationQ.x, EndRotationQ.y, EndRotationQ.z, EndRotationQ.w);
	LOG->Info(LOG_LOW, "rot: Out:   %.3f, %.3f, %.3f, %.3f", Out.x, Out.y, Out.z, Out.w);
	*/
}

void Model::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

const aiNodeAnim* Model::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}