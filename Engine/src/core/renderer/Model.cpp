﻿// Model.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/Texture.h"
#include "core/renderer/Shader.h"
#include "core/renderer/Mesh.h"
#include "core/renderer/Model.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

namespace Phoenix {

	// For converting between ASSIMP and glm
	static inline glm::vec3 vec3_cast(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
	static inline glm::vec2 vec2_cast(const aiVector3D& v) { return glm::vec2(v.x, v.y); }
	static inline glm::quat quat_cast(const aiQuaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }
	static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }
	static inline glm::mat4 mat4_cast(const aiMatrix3x3& m) { return glm::transpose(glm::make_mat3(&m.a1)); }

	Model::Model()
		:
		m_matProjection(glm::mat4(1)),
		m_matView(glm::mat4(1)),
		m_matBaseModel(glm::mat4(1)),
		m_matMVP(glm::mat4(1)),
		m_matPrevProjection(glm::mat4(1)),
		m_matPrevView(glm::mat4(1)),
		m_matPrevMVP(glm::mat4(1)),
		m_matGlobalInverseTransform(glm::mat4(1)),
		m_statNumAnimations(0),
		m_numBones(0),
		m_statNumCameras(0),
		m_statNumMeshes(0),
		m_statNumVertices(0),
		m_statNumFaces(0),
		m_statNumBones(0),
		m_pScene(nullptr),
		playAnimation(false),		// By default, animations are disabled
		useCamera(false),			// By default, we don't use the model camera
		m_currentAnimation(0),
		m_currentCamera(0),
		m_animDuration(0),
		m_bLoadedUniqueVertices(false)
	{
	}

	Model::~Model()
	{
		meshes.clear();
		directory = "";
		filename = "";
		filepath = "";
	}

	void Model::Draw(SP_Shader shader, float currentTime, uint32_t startTexUnit)
	{
		// Load the model transformation on all sub-meshes
		setMeshesModelTransform();

		// Set the Bones transformations and send the Bones info to the Shader (gBones uniform)
		if (playAnimation)
			setBoneTransformations(shader, currentTime);

		// If we use camera, override the matrix view for the camera view
		if (useCamera) {
			if ((m_currentCamera >= 0) && (m_currentCamera < m_camera.size())) {
				m_matView = m_camera[m_currentCamera]->getView();
			}
		}

		// Send the matrices
		shader->setValue("projection", m_matProjection);// TODO: Not to be stored here: What happens if we are drawing multiple instances of the same object?
		shader->setValue("view", m_matView);

		// Find Shader and do the matrix stuff
		shader->setValue("prev_projection", m_matPrevProjection);// TODO: Not to be stored here: What happens if we are drawing multiple instances of the same object?
		shader->setValue("prev_view", m_matPrevView);

		// Then, send the model matrice of each mesh and draw them
		for (auto& mesh : meshes) {
			// Send model matrix
			shader->setValue("model", mesh->m_matModel);
			m_matMVP = m_matProjection * m_matView * mesh->m_matModel;
			shader->setValue("MVP", m_matMVP);

			// Send previous model matrix
			shader->setValue("prev_model", mesh->m_matPrevModel);
			m_matPrevMVP = m_matPrevProjection * m_matPrevView * mesh->m_matPrevModel;
			shader->setValue("prev_MVP", m_matPrevMVP);
			mesh->m_matPrevModel = mesh->m_matModel;

			// Draw
			mesh->Draw(shader, startTexUnit);
		}

		m_matPrevProjection = m_matProjection;
		m_matPrevView = m_matView;
	}

	void Model::setAnimation(unsigned int a)
	{
		if (a < m_statNumAnimations) {
			m_currentAnimation = a;
			m_animDuration = m_pScene->mAnimations[m_currentAnimation]->mDuration; // Load anim duration
		}
		else
			Logger::error("The animation number [{}] is not available in the file [{}]", a, filename);
	}

	void Model::setCamera(unsigned int c)
	{
		if (c < m_statNumCameras) {
			useCamera = true;
			m_currentCamera = c;
		}
		else {
			useCamera = false;
			m_currentCamera = 0;
			Logger::error("The camera number [{}] is not available in the file [{}]", c, filename);
		}
	}

	void Model::loadUniqueVertices()
	{
		if (!m_bLoadedUniqueVertices) {
			for (auto& mesh : meshes) {
				mesh->loadUniqueVerticesPos();
			}
			m_bLoadedUniqueVertices = true;
		}
	}

	// Returns false if model has not been properly loaded
	bool Model::Load(std::string_view path)
	{
		filepath = path;

		// Load file
		unsigned int importerFlags =
			aiProcess_CalcTangentSpace |			// Calc tangents and bitangents
			aiProcess_Triangulate |					// Force all faces to be triangles
			aiProcess_GenSmoothNormals |			// Generate Smooth normals
			aiProcess_FlipUVs |						// Flip UV's coordinates
			aiProcess_LimitBoneWeights |			// limit bone weights to 4 per vertex
			aiProcess_JoinIdenticalVertices |		// JoinIdenticalVertices does not work? https://github.com/assimp/assimp/issues/2006| --> This reduces the geometry a lot!
			aiProcess_GenUVCoords |					// convert spherical, cylindrical, box and planar mapping to proper UVs
			aiProcess_TransformUVCoords |			// preprocess UV transformations (scaling, translation ...)
			aiProcess_OptimizeMeshes |				// join small meshes, if possible
			aiProcess_OptimizeGraph |				// Optimize hierarchy
			aiProcess_SplitByBoneCount |			// split meshes with too many bones. Necessary for our (limited) hardware skinning shader
			aiProcess_FindInstances |				// search for instanced meshes and remove them by references to one master
			aiProcess_ValidateDataStructure |		// perform a full validation of the loader's output
			aiProcess_ImproveCacheLocality |		// improve the cache locality of the output vertices
			aiProcess_RemoveRedundantMaterials |	// remove redundant materials
			aiProcess_FindDegenerates |				// remove degenerated polygons from the import
			//aiProcess_FindInvalidData |			// detect invalid model data, such as invalid normal vectors --> Sometimes this give errors on models with animations
			0;

		m_pScene = m_Importer.ReadFile(filepath, importerFlags);

		// check for errors
		if (!m_pScene || m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_pScene->mRootNode) // if is Not Zero
		{
			Logger::error("Error loading file [{}]: {}", filepath, m_Importer.GetErrorString());
			return false;
		}
		// retrieve the directory path of the filepath and the filename (without the data folder, because each loader adds the data folder)
		directory = filepath.substr(0, filepath.find_last_of('/'));
		filename = filepath.substr(filepath.find_last_of('/') + 1, filepath.length());
		Logger::info(LogLevel::low, "Loading Model: {}", filename);

		// Get transformation matrix for nodes (vertices relative to bones)
		m_matGlobalInverseTransform = mat4_cast(m_pScene->mRootNode->mTransformation);
		m_matGlobalInverseTransform = glm::inverse(m_matGlobalInverseTransform);
		// process ASSIMP's root node recursively
		processNode(m_pScene->mRootNode, m_pScene);

		// Get the cameras
		processCameras(m_pScene);

		// COmpute the stats of the Model
		getStats();

		return true;
	}

	// Processes scene cameras
	void Model::processCameras(const aiScene* scene)
	{
		m_statNumCameras = m_pScene->mNumCameras;
		for (unsigned int i = 0; i < m_pScene->mNumCameras; i++)
		{
			aiCamera* aiCam = m_pScene->mCameras[i];

			//glm::vec3 cam_pos = glm::vec3(aiCam->mPosition.x, aiCam->mPosition.y, aiCam->mPosition.z);
			//glm::vec3 cam_lookAt = glm::vec3(aiCam->mLookAt.x, aiCam->mLookAt.y, aiCam->mLookAt.z);
			//glm::vec3 cam_up = glm::vec3(aiCam->mUp.x, aiCam->mUp.y, aiCam->mUp.z);
			// TODO: Read FOV, YAW and PITCH

			aiMatrix4x4 mat;
			aiCam->GetCameraMatrix(mat);
			Camera* cam = new CameraRawMatrix(mat4_cast(mat));
			cam->TypeStr = aiCam->mName.C_Str();
			m_camera.emplace_back(cam);
		}
	}


	void Model::getStats()
	{
		// Count total number of meshes and their vertices
		m_statNumMeshes = static_cast<unsigned int>(meshes.size());
		m_statNumVertices = 0;
		m_statNumFaces = 0;
		for (uint32_t i = 0; i < m_statNumMeshes; i++) {
			m_statNumVertices += meshes[i]->m_numVertices;
			m_statNumFaces += meshes[i]->m_numFaces;
		}
			

		// Count total number of animations
		m_statNumAnimations = m_pScene->mNumAnimations;

		m_statNumCameras = m_pScene->mNumCameras;

		m_statNumBones = m_numBones;
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any)
	void Model::processNode(aiNode* node, const aiScene* scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			Logger::info(LogLevel::low, "Reading node name: {}", node->mName.data);
			meshes.emplace_back(processMesh(node->mName.data, mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}


	SP_Mesh Model::processMesh(std::string nodeName, aiMesh* mesh, const aiScene* scene)
	{
		// data to fill
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		Logger::info(LogLevel::low, "Loading mesh: {}", mesh->mName.C_Str());

		if (mesh->HasNormals() == false)
			Logger::error("The loaded mesh has no Normal info.");

		if (mesh->HasTangentsAndBitangents() == false)
			Logger::info(LogLevel::med, "Warning, the loaded mesh has no Tangents and BiTangents! Normal will be copied there.");

		vertices.reserve(mesh->mNumVertices);			// Allocate memory so we avoid resizing the vector each time
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
			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			else
				vertex.Normal = glm::vec3(0, 0, 0);

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
				vertex.TexCoords = glm::vec2(0, 0);
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

			vertices.emplace_back(vertex);
		}

		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		uint32_t numTotalIndices = 0;
		for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
			numTotalIndices += mesh->mFaces[i].mNumIndices;
		}

		// Allocate memory so we avoid resizing the vector each time
		indices.reserve(numTotalIndices);
				
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];

			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.emplace_back(face.mIndices[j]);
		}

		// Process Bones of this mesh and loads it into the "BoneInfo" and "BoneMapping" lists
		// BoneInfo and BoneMapping are like an indexed database of bones (TODO: Put all the Bones stuff in a Class)
		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName(mesh->mBones[boneIndex]->mName.C_Str());

			if (m_boneInfoMap.find(boneName) == m_boneInfoMap.end())
			{
				// Allocate an index for the new bone
				BoneInfo bi;
				bi.id = m_numBones;
				bi.BoneOffset = mat4_cast(mesh->mBones[boneIndex]->mOffsetMatrix);
				// Store the bone mapping: Bone Name + Bone Position in array
				m_boneInfoMap[boneName] = bi;
				boneID = m_numBones;
				m_numBones++;

				// Allocate space for the bone transformation matrix
				m_boneTransforms.emplace_back(glm::mat4(1.0f));
			}
			else
			{
				boneID = m_boneInfoMap[boneName].id;
			}

			auto weights = mesh->mBones[boneIndex]->mWeights;
			auto numWeights = mesh->mBones[boneIndex]->mNumWeights;
			
			for (uint32_t weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				unsigned int VertexID = weights[weightIndex].mVertexId;
				float Weight = weights[weightIndex].mWeight;
				vertices[VertexID].Bone.AddBoneData(boneID, Weight);
			}
		}

		// Process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// return a mesh object created from the extracted mesh data
		return std::make_shared<Mesh>(
			scene,
			nodeName,
			mesh,
			vertices,
			indices,
			material,
			directory,
			filename
			);
	}

	void Model::setMeshesModelTransform()
	{
		for (auto& mesh : meshes)
			mesh->m_matModel = m_matBaseModel;
	}

	/////////////// Bones calculations
	// TODO: Do a Bones Class, with all this calculations
	void Model::setBoneTransformations(SP_Shader shader, float currentTime)
	{
		// TODO: Hacer que los Transforms sea una propiedad de cada uno del Bone, asi no se tiene q hacer
		// Resize ni cosas raras, y asi ya tenemos un vector de transformaciones de tamaño fijo
		if (m_pScene->HasAnimations()) {
			boneTransform(currentTime);
			if (m_boneTransforms.size() > 0)
				shader->setValue("gBones", m_boneTransforms[0], (GLsizei)m_boneTransforms.size());
		}
	}


	void Model::boneTransform(float timeInSeconds)
	{
		float TicksPerSecond = (float)(m_pScene->mAnimations[m_currentAnimation]->mTicksPerSecond != 0 ?
			m_pScene->mAnimations[m_currentAnimation]->mTicksPerSecond : 25.0f);
		float TimeInTicks = timeInSeconds * TicksPerSecond;
		float AnimationTime = (float)fmod(TimeInTicks, m_animDuration);

		ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, glm::mat4(1.0f));

		// TODO: There should be anothr way to do this, although it works :)
		for (auto const& bone : m_boneInfoMap) {
			m_boneTransforms[bone.second.id] = bone.second.FinalTransformation;
		}
	}

	void Model::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
	{
		std::string NodeName(pNode->mName.data);

		const aiAnimation* pAnimation = m_pScene->mAnimations[m_currentAnimation];

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
			/*
			// Show debug info
			Logger::info(LogLevel::low, "Node Anim: %s, second: %.3f", pNodeAnim->mNodeName.data, AnimationTime);
			Logger::info(LogLevel::low, "S: %.3f, %.3f, %.3f", scale.x, scale.y, scale.z);
			Logger::info(LogLevel::low, "R: %.3f, %.3f, %.3f, %.3f", RotationQ.x, RotationQ.y, RotationQ.z, RotationQ.w);
			Logger::info(LogLevel::low, "T: %.3f, %.3f, %.3f", translation.x, translation.y, translation.z);
			*/
			// Combine the above transformations
			NodeTransformation = TranslationM * RotationM * ScalingM;
		}

		// Combine with node Transformation with Parent Transformation
		glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;


		// Now we need to apply the Matrix to the corresponding object
		for (unsigned int i = 0; i < m_statNumMeshes; i++) {
			if (NodeName == meshes[i]->m_nodeName) {
				meshes[i]->m_matModel *= m_matGlobalInverseTransform * GlobalTransformation;
				/*Logger::info(LogLevel::low, "Aqui toca guardar la matriz, para el objeto: %s, que es la mesh: %boneIndex [time: %.3f]", NodeName.c_str(), boneIndex, AnimationTime);
				glm::mat4 M = GlobalTransformation;
				Logger::info(LogLevel::low, "M: [%.2f, %.2f, %.2f, %.2f], [%.2f, %.2f, %.2f, %.2f], [%.2f, %.2f, %.2f, %.2f], [%.2f, %.2f, %.2f, %.2f]",
					M[0][0], M[0][1], M[0][2], M[0][3],
					M[1][0], M[1][1], M[1][2], M[1][3],
					M[2][0], M[2][1], M[2][2], M[2][3],
					M[3][0], M[3][1], M[3][2], M[3][3]);
					*/
			}
		}

		for (unsigned int i = 0; i < m_pScene->mNumCameras; i++) {
			if (NodeName == m_camera[i]->TypeStr) {
				m_camera[i]->setViewMatrix (glm::inverse(GlobalTransformation));
			}
		}

		if (m_boneInfoMap.find(NodeName) != m_boneInfoMap.end()) {
			m_boneInfoMap[NodeName].FinalTransformation = m_matGlobalInverseTransform * GlobalTransformation * m_boneInfoMap[NodeName].BoneOffset;
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
		/*Logger::info(LogLevel::low, "rot: factor: %.3f", Factor);
		Logger::info(LogLevel::low, "rot: Start: %.3f, %.3f, %.3f, %.3f", StartRotationQ.x, StartRotationQ.y, StartRotationQ.z, StartRotationQ.w);
		Logger::info(LogLevel::low, "rot: End:   %.3f, %.3f, %.3f, %.3f", EndRotationQ.x, EndRotationQ.y, EndRotationQ.z, EndRotationQ.w);
		Logger::info(LogLevel::low, "rot: Out:   %.3f, %.3f, %.3f, %.3f", Out.x, Out.y, Out.z, Out.w);
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
}