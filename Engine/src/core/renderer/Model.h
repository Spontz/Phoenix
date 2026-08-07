// Model.h
// Spontz Demogroup


#pragma once

#include "main.h"
#include "core/Demokernel.h"
#include "core/renderer/Mesh.h"
#include "core/renderer/Shader.h"
#include "core/renderer/Camera.h"

#include <optional>

namespace Phoenix {

	class Model;
	using SP_Model = std::shared_ptr<Model>;
	using WP_Model = std::weak_ptr<Model>;

	class Model final
	{
	public:
		// State of the model camera resolved by the precalculation phase.
		// Sections read this between PreCalc() and their expression evaluation.
		struct ModelCameraState final {
			bool		valid = false;					// Is a model camera currently selected and available?
			glm::mat4	view = glm::mat4(1.0f);			// View matrix of the resolved camera
			glm::vec3	position = glm::vec3(0.0f);
			glm::vec3	front = glm::vec3(0.0f, 0.0f, -1.0f);
			glm::vec3	up = glm::vec3(0.0f, 1.0f, 0.0f);
			float		yaw = 0.0f;
			float		pitch = 0.0f;
			float		roll = 0.0f;
			float		fov = 0.0f;
		};

	public:
		Model();
		~Model();

	public:
		// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
		bool Load(std::string_view path);
		// Precalculation phase: resolves the animated state of the model (node hierarchy, bone
		// transformations, per-mesh animated transforms and model camera views) for the given
		// playback time. Issues no GPU work, so sections can read the result - notably the model
		// camera - before evaluating their expressions and drawing.
		// Must be called before Draw() on every frame the model is rendered.
		void PreCalc(float currentTime);
		// draws the model, and thus all its meshes, using the state resolved by PreCalc()
		void Draw(SP_Shader shader, uint32_t startTexUnit = 0);
		// Render-phase helper: uploads the bone transformations resolved by PreCalc() to the
		// shader (gBones uniform). Performs no computation.
		void uploadBoneTransforms(SP_Shader shader);
		void setAnimation(unsigned int a);
		void setCamera(unsigned int c);

		// Camera resolved by the last PreCalc() call
		const ModelCameraState& getModelCamera() const { return m_modelCamera; }

		// Load unique vertices
		void loadUniqueVertices();

	private:
		// Get Stats from the model
		void getStats();

		// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
		void processNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
		SP_Mesh processMesh(std::string nodeName, aiMesh* mesh, const glm::mat4& nodeGlobalTransform);

		// Process the scene cameras
		void processCameras(const aiScene* scene);

		// Capture the state of the currently selected model camera
		void resolveModelCamera();

		// Bones Calculations
	private:
		void boneTransform(float timeInSeconds);
		// Bones Transformations
		void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
		unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
		const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
		void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);

	public:
		std::vector<SP_Mesh>	meshes;
		std::string				directory;			// Path of the model file
		std::string				filename;			// Name of the model file
		std::string				filepath;			// Full path of the model file
		bool					playAnimation;		// Do we want to compute the transformations for playing animations?
		bool					useCamera;			// Do we want to use the camera of the model?

		// Matrices
		glm::mat4				m_matProjection;	// Projection matrix
		glm::mat4				m_matView;			// View matrix
		glm::mat4				m_matBaseModel;		// Base model matrix (the intial matrix of all sub-meshes)
		glm::mat4				m_matMVP;			// Model-View-Projection matrix

		glm::mat4				m_matPrevProjection;// Projection matrix
		glm::mat4				m_matPrevView;		// View matrix
		glm::mat4				m_matPrevMVP;		// Model-View-Projection matrix

		std::vector<Camera*>	m_camera;

		// Stats
		uint32_t				m_statNumVertices;
		uint32_t				m_statNumFaces;
		uint32_t				m_statNumMeshes;
		uint32_t				m_statNumCameras;
		uint32_t				m_statNumAnimations;
		uint32_t				m_statNumBones;

	private:
		Assimp::Importer		m_Importer;
		const aiScene*			m_pScene;
		std::vector<Material>	m_materials;
		glm::mat4				m_matGlobalInverseTransform;// Global transformation matrix for nodes (vertices relative to bones)
		// Bones info
		std::map<std::string, BoneInfo>		m_boneInfoMap;				// maps a bone name to its information
		std::vector<glm::mat4>				m_boneTransforms;			// Bone transformations
		uint32_t							m_numBones;
		unsigned int						m_currentCamera;			// Current Camera
		unsigned int						m_currentAnimation;			// Current Animation
		ModelCameraState					m_modelCamera;				// Model camera state resolved by PreCalc()
		double								m_animDuration;				// Animation duration in seconds
		bool								m_bLoadedUniqueVertices;	// Have we loaded the unique vertices for each mesh?
		// Last values already reported to the log, to avoid flooding it from the per-frame setters
		std::optional<unsigned int>			m_lastInvalidAnimation;
		std::optional<unsigned int>			m_lastInvalidCamera;
		std::optional<unsigned int>			m_lastInvalidAnimDuration;
	};
}
