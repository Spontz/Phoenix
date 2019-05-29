#include "skinnedmesh.hpp"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtx\string_cast.hpp>
#include <glm\gtx\quaternion.hpp>


#define POSITION_LOCATION    0
#define NORMAL_LOCATION      1
#define TEX_COORD_LOCATION   2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

// For converting between ASSIMP and glm
static inline glm::vec3 vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::vec2 vec2_cast(const aiVector3D &v) { return glm::vec2(v.x, v.y); } // it's aiVector3D because assimp's texture coordinates use that
static inline glm::quat quat_cast(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }
static inline glm::mat4 mat4_cast(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
static inline glm::mat4 mat4_cast(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }



void SkinnedMesh::setBoneTransformations(GLuint shaderProgram, GLfloat currentTime)
{
	if (m_pScene->HasAnimations()) {
		std::vector<glm::mat4> Transforms;
		boneTransform((float)currentTime, Transforms);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "gBones"), (GLsizei)Transforms.size(), GL_FALSE, glm::value_ptr(Transforms[0]));
	}
}

/* Adds a new Bone */
void SkinnedMesh::VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
	for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
		if (Weights[i] == 0.0)
		{
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
}

/* Init */
SkinnedMesh::SkinnedMesh():currentAnimation(0)
{
	m_VAO = 0;
	for (unsigned int i = 0; i < NUM_VBs; ++i)
	{
		m_Buffers[i] = 0;
	}
	m_NumBones = 0;
	m_pScene = NULL;
}

SkinnedMesh::~SkinnedMesh()
{
	Clear();
}

void SkinnedMesh::Clear()
{
	/* Textures must not be deleted because they are GLuint */

	/* Deletes VBOs */
	if (m_Buffers[0] != 0)
		glDeleteBuffers(NUM_VBs, m_Buffers);
	/* Deletes VAO */
	if (m_VAO != 0)
	{
		glDeleteVertexArrays(1, &m_VAO);
		m_VAO = 0;
	}
}

/* loads the model */
bool SkinnedMesh::loadMesh(const std::string& fileName)
{
	/* Deletes the previous loaded mesh(if it exists) */
	Clear();

	/* Create the VAO */
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	/* Create VBOs for vertices attributes */
	glGenBuffers(NUM_VBs, m_Buffers);

	/* Return value */
	bool ret = false;

	m_pScene = m_Importer.ReadFile(fileName.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (m_pScene)
	{
		/* Get transformation matrix for nodes(vertices relative to boes) */
		m_GlobalInverseTransform = mat4_cast(m_pScene->mRootNode->mTransformation);
		m_GlobalInverseTransform =  glm::inverse(m_GlobalInverseTransform);
		
		ret = InitFromScene(m_pScene, fileName);
	}
	else
	{
		std::cout << "Error parsing : " << fileName << " : " << m_Importer.GetErrorString() << std::endl;
		return false;
	}

	/* Make sure the VAO is not changed from the outside */
	glBindVertexArray(0);

	return ret;
}

unsigned int SkinnedMesh::getNumAnimations()
{
	return m_pScene->mNumAnimations;
}

void SkinnedMesh::setAnimation(unsigned int a)
{
	if (a >= 0 && a < getNumAnimations())
		currentAnimation = a;
}

bool SkinnedMesh::InitFromScene(const aiScene* pScene, const std::string& fileName)
{
	/* Resize the mesh & texture vectors */
	m_Entries.resize(pScene->mNumMeshes);
	m_Textures.resize(pScene->mNumMaterials);
	
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec2> TexCoords;
	std::vector<VertexBoneData> Bones;
	std::vector<unsigned int> Indices;

	unsigned int NumVertices = 0;
	unsigned int NumIndices = 0;

	/* Count the number of vertices and indices */
	for (unsigned int i = 0; i < m_Entries.size(); ++i)
	{
		m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
		m_Entries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
		m_Entries[i].BaseVertex = NumVertices;
		m_Entries[i].BaseIndex = NumIndices;

		NumVertices += pScene->mMeshes[i]->mNumVertices;
		NumIndices += m_Entries[i].NumIndices;
	}

	// Reserve space in the vectors for the vertex attributes and indices
	Positions.reserve(NumVertices);
	Normals.reserve(NumVertices);
	TexCoords.reserve(NumVertices);
	Bones.resize(NumVertices);
	Indices.reserve(NumIndices);

	/* Initialize the meshes in the scene one by one */
	for (unsigned int i = 0; i < m_Entries.size(); ++i)
	{
		/* get mesh */
		const aiMesh* paiMesh = pScene->mMeshes[i];
		/* init the mesh */
		InitMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
	}
	
	/* init the material */
	if (!InitMaterials(pScene, fileName))
		return false;
	
	/* Generate and populate the buffers with vertex attributes and indices */
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEX_COORD_LOCATION);
	glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	if (glGetError() == GL_NO_ERROR)
		return true;
	else
		return false;
}

void SkinnedMesh::InitMesh(unsigned int MeshIndex,
	const aiMesh* paiMesh,
	std::vector<glm::vec3>& Positions,
	std::vector<glm::vec3>& Normals,
	std::vector<glm::vec2>& TexCoords,
	std::vector<VertexBoneData>& Bones,
	std::vector<unsigned int>& Indices)
{
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

	std::cout << "Num vertices = " << paiMesh->mNumVertices << std::endl;
	/* Populize the vertex attribute vectors */
	for (unsigned int i = 0; i < paiMesh->mNumVertices; ++i)
	{
		/* Get pos normal texCoord */
		
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		
		const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

		/* store pos normal texCoord */
		Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
	
		if (paiMesh->HasNormals())
		{
			const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
			Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
		}
		TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));
	}

	/* Load bones */
	LoadBones(MeshIndex, paiMesh, Bones);

	/* Populate the index buffer */
	for (unsigned int i = 0; i < paiMesh->mNumFaces; ++i)
	{
		const aiFace& Face = paiMesh->mFaces[i];
		/* if mNumIndices != 3 then worning */
		assert(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}
}

void SkinnedMesh::LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
	/* Load bones one by one */
	
	for (unsigned int i = 0; i < pMesh->mNumBones; ++i)
	{
		unsigned int BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end())
		{
			/* allocate an index for the new bone */
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);

			m_BoneInfo[BoneIndex].BoneOffset = mat4_cast(pMesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else
		{
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; ++j)
		{
			//std::cout << pMesh->mBones[i]->mWeights. << std::endl;
			unsigned int VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}
}

bool SkinnedMesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
	// Extract the directory part from the file name
	std::string::size_type SlashIndex = Filename.find_last_of("/");
	std::string Dir;

	if (SlashIndex == std::string::npos) {
		Dir = ".";
	}
	else if (SlashIndex == 0) {
		Dir = "/";
	}
	else {
		Dir = Filename.substr(0, SlashIndex);
	}

	bool ret = true;

	/* Initialize the materials */
	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		/* Get the material */
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		m_Textures[i] = 0;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString Path;

			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				std::string p(Path.data);

				if (p.substr(0, 2) == ".\\")
				{
					p = p.substr(2, p.size() - 2);
				}

				std::string FullPath = Dir + "/" + p;

				

				/* Here load the textures */
			}
		}
	}
	return ret;
}

void SkinnedMesh::render(const GLuint& shaderProgram)
{
	glBindVertexArray(m_VAO);

	for (unsigned int i = 0; i < m_Entries.size(); i++) {
		const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;

		assert(MaterialIndex < m_Textures.size());

		//if (m_Textures[MaterialIndex]) {
		//	glActiveTexture(GL_TEXTURE0);
		//	glBindTexture(GL_TEXTURE_2D, m_Textures[MaterialIndex]);


			/* the texture uniform */
			//GLint textureLocation = glGetUniformLocation(shaderProgram, "texture1");
			//glUniform1i(textureLocation, 0);
		//}

		glDrawElementsBaseVertex(GL_TRIANGLES,
			m_Entries[i].NumIndices,
			GL_UNSIGNED_INT,
			(void*)(sizeof(unsigned int) * m_Entries[i].BaseIndex),
			m_Entries[i].BaseVertex);
	}

	glBindVertexArray(0);
}

unsigned int SkinnedMesh::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

unsigned int SkinnedMesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

unsigned int SkinnedMesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

void SkinnedMesh::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

void SkinnedMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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
}

void SkinnedMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

void SkinnedMesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[currentAnimation];

	glm::mat4 NodeTransformation = mat4_cast(pNode->mTransformation);
	
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

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM *ScalingM;
	}
	
	// Combine with node Transformation with Parent Transformation
	glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
		unsigned int BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

void SkinnedMesh::boneTransform(float timeInSeconds, std::vector<glm::mat4>& Transforms)
{
	glm::mat4 Identity = glm::mat4(1.0f);
	
	//TODO: I think that this line does not make any sense... because its overwritten later
	animDuration = (float)m_pScene->mAnimations[currentAnimation]->mDuration;

	/* Calc animation duration */
	unsigned int numPosKeys = m_pScene->mAnimations[currentAnimation]->mChannels[0]->mNumPositionKeys;
	animDuration = m_pScene->mAnimations[currentAnimation]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;

	float TicksPerSecond = (float)(m_pScene->mAnimations[currentAnimation]->mTicksPerSecond != 0 ? m_pScene->mAnimations[currentAnimation]->mTicksPerSecond : 25.0f);
	float TimeInTicks = timeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, animDuration);

	ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

	Transforms.resize(m_NumBones);
	
	for (unsigned int  i = 0; i < m_NumBones; i++) {
		Transforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}

const aiNodeAnim* SkinnedMesh::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}
