#include "main.h"
#include "core/renderer/ParticleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

#include <algorithm>
#include <iomanip>
#include <random>

namespace Phoenix {

	class sDrawParticleMorphing final : public Section {
	public:
		sDrawParticleMorphing();
		~sDrawParticleMorphing();

	public:
		bool		load();
		void		init();
		void		warmExec();
		void		exec();
		void		loadDebugStatic();
		std::string debug();

	private:
		// 3D Scenes (source and destination)
		SP_Model	m_pModelSource;
		int			m_iModelSourceUniqueVerticesNum = 0;
		SP_Model	m_pModelDest;
		int			m_iModelDestUniqueVerticesNum = 0;

		// Particle engine variables
		int				m_iNumParticles = 0;
		float			m_fDuration = 0.0f;
		ParticleMesh*	m_pParticleMesh = nullptr;
		SP_Shader		m_pShader = nullptr;

		// Particle Matrix positioning (applied to both source and destination)
		glm::vec3		m_vTranslation = { 0, 0, 0 };
		glm::vec3		m_vRotation = { 0, 0, 0 };
		glm::vec3		m_vScale = { 1, 1, 1 };

		MathDriver*		m_pExprPosition = nullptr;	// Equation for positioning the object
		ShaderVars*		m_pVars = nullptr;			// For storing any other shader variables
	};

	// ******************************************************************

	Section* instance_drawParticleMorphing()
	{
		return new sDrawParticleMorphing();
	}

	sDrawParticleMorphing::sDrawParticleMorphing()
	{
		type = SectionType::DrawParticleMorphing;
	}

	sDrawParticleMorphing::~sDrawParticleMorphing()
	{
		if (m_pExprPosition)
			delete m_pExprPosition;
		if (m_pVars)
			delete m_pVars;
		if (m_pParticleMesh)
			delete m_pParticleMesh;
	}

	// Collect all unique vertices across all meshes of a model into a single flat list
	static std::vector<glm::vec3> collectUniqueVertices(const SP_Model& model)
	{
		std::vector<glm::vec3> verts;
		if (!model)
			return verts;
		size_t total = 0;
		for (auto& mesh : model->meshes)
			total += mesh->m_uniqueVertices.size();
		verts.reserve(total);
		for (auto& mesh : model->meshes)
			for (auto& uv : mesh->m_uniqueVertices)
				verts.push_back(uv.Position);
		return verts;
	}

	struct SurfaceTriangle
	{
		glm::vec3 a;
		glm::vec3 b;
		glm::vec3 c;
		float area;
	};

	static std::vector<SurfaceTriangle> collectSurfaceTriangles(const SP_Model& model)
	{
		std::vector<SurfaceTriangle> triangles;
		if (!model)
			return triangles;

		constexpr float minTriangleArea = 1e-8f;
		for (const auto& mesh : model->meshes) {
			const auto& vertices = mesh->getVertices();
			const auto& indices = mesh->getIndices();
			for (size_t i = 0; i + 2 < indices.size(); i += 3) {
				const unsigned int indexA = indices[i];
				const unsigned int indexB = indices[i + 1];
				const unsigned int indexC = indices[i + 2];
				if (indexA >= vertices.size() || indexB >= vertices.size() || indexC >= vertices.size())
					continue;

				const glm::vec3& a = vertices[indexA].Position;
				const glm::vec3& b = vertices[indexB].Position;
				const glm::vec3& c = vertices[indexC].Position;
				const float area = 0.5f * glm::length(glm::cross(b - a, c - a));
				if (area > minTriangleArea)
					triangles.push_back({ a, b, c, area });
			}
		}

		return triangles;
	}

	// Keeps the ordered unique-vertex prefix intact, then fills only the surplus with
	// deterministic, area-weighted points in triangle interiors.
	static bool sampleVertexFirst(const SP_Model& model, const std::vector<glm::vec3>& vertices, size_t count, std::vector<glm::vec3>& out)
	{
		out.clear();
		out.reserve(count);
		if (vertices.empty() || count == 0)
			return false;

		const size_t vertexCount = std::min(count, vertices.size());
		out.insert(out.end(), vertices.begin(), vertices.begin() + vertexCount);
		if (out.size() == count)
			return true;

		const std::vector<SurfaceTriangle> triangles = collectSurfaceTriangles(model);
		if (triangles.empty())
			return false;

		std::vector<float> triangleAreas;
		triangleAreas.reserve(triangles.size());
		for (const auto& triangle : triangles)
			triangleAreas.push_back(triangle.area);

		std::mt19937 randomGenerator(static_cast<uint32_t>(0x5D12F3A7u ^ count));
		std::discrete_distribution<size_t> triangleDistribution(triangleAreas.begin(), triangleAreas.end());
		constexpr float barycentricEpsilon = 1e-6f;
		std::uniform_real_distribution<float> randomUnit(barycentricEpsilon, 1.0f - barycentricEpsilon);

		while (out.size() < count) {
			const SurfaceTriangle& triangle = triangles[triangleDistribution(randomGenerator)];
			const float sqrtU = glm::sqrt(randomUnit(randomGenerator));
			const float v = randomUnit(randomGenerator);
			const float weightA = 1.0f - sqrtU;
			const float weightB = sqrtU * (1.0f - v);
			const float weightC = sqrtU * v;
			out.push_back(weightA * triangle.a + weightB * triangle.b + weightC * triangle.c);
		}

		return true;
	}

	bool sDrawParticleMorphing::load()
	{
		// script validation:
		// param[0] = number of particles
		// param[1] = duration (in seconds) of the morphing
		// strings[0] = source scene model
		// strings[1] = destination scene model
		// shaderBlock[0] = shader
		// expressionRun = optional positioning expression (tx,ty,tz,rx,ry,rz,sx,sy,sz)
		if ((param.size() != 2) || (strings.size() != 2) || (shaderBlock.size() != 1)) {
			Logger::error("Draw Particle Morphing [{}]: Contract mismatch. Expected 2 params (NumParticles, MorphingDuration), 2 strings (SourceModel, DestinationModel) and 1 shader block", identifier);
			return false;
		}

		// Set render states
		render_disableDepthMask = true;

		// Load config
		m_iNumParticles = static_cast<int>(param[0]);
		m_fDuration = param[1];

		if (m_iNumParticles <= 0) {
			Logger::error("Draw Particle Morphing [{}]: Number of particles must be greater than 0", identifier);
			return false;
		}
		if (m_fDuration <= 0.0f) {
			Logger::error("Draw Particle Morphing [{}]: Morphing duration must be greater than 0", identifier);
			return false;
		}

		// Load the shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		// Load source and destination models
		m_pModelSource = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0]);
		m_pModelDest = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1]);
		if (!m_pModelSource) {
			Logger::error("Draw Particle Morphing [{}]: Failed to load source model '{}'", identifier, strings[0]);
			return false;
		}
		if (!m_pModelDest) {
			Logger::error("Draw Particle Morphing [{}]: Failed to load destination model '{}'", identifier, strings[1]);
			return false;
		}

		// Load unique vertices for both models
		m_pModelSource->loadUniqueVertices();
		m_pModelDest->loadUniqueVertices();

		// Collect all unique vertices from both scenes
		std::vector<glm::vec3> srcVerts = collectUniqueVertices(m_pModelSource);
		m_iModelSourceUniqueVerticesNum = static_cast<int>(srcVerts.size());
		std::vector<glm::vec3> dstVerts = collectUniqueVertices(m_pModelDest);
		m_iModelDestUniqueVerticesNum = static_cast<int>(dstVerts.size());

		if (srcVerts.empty()) {
			Logger::error("Draw Particle Morphing [{}]: No vertices found in the source model", identifier);
			return false;
		}
		if (dstVerts.empty()) {
			Logger::error("Draw Particle Morphing [{}]: No vertices found in the destination model", identifier);
			return false;
		}

		// Sample source and destination independently, preserving their vertex prefixes.
		std::vector<glm::vec3> srcSamples;
		if (!sampleVertexFirst(m_pModelSource, srcVerts, static_cast<size_t>(m_iNumParticles), srcSamples)) {
			Logger::error("Draw Particle Morphing [{}]: Source model has no valid triangles for {} fallback particles", identifier, m_iNumParticles - static_cast<int>(srcVerts.size()));
			return false;
		}

		std::vector<glm::vec3> dstSamples;
		if (!sampleVertexFirst(m_pModelDest, dstVerts, static_cast<size_t>(m_iNumParticles), dstSamples)) {
			Logger::error("Draw Particle Morphing [{}]: Destination model has no valid triangles for {} fallback particles", identifier, m_iNumParticles - static_cast<int>(dstVerts.size()));
			return false;
		}

		// Load particle positioning expression
		m_pExprPosition = new MathDriver(this);
		m_pExprPosition->expression = expressionRun;

		m_pExprPosition->SymbolTable.add_variable("tx", m_vTranslation.x);
		m_pExprPosition->SymbolTable.add_variable("ty", m_vTranslation.y);
		m_pExprPosition->SymbolTable.add_variable("tz", m_vTranslation.z);
		m_pExprPosition->SymbolTable.add_variable("rx", m_vRotation.x);
		m_pExprPosition->SymbolTable.add_variable("ry", m_vRotation.y);
		m_pExprPosition->SymbolTable.add_variable("rz", m_vRotation.z);
		m_pExprPosition->SymbolTable.add_variable("sx", m_vScale.x);
		m_pExprPosition->SymbolTable.add_variable("sy", m_vScale.y);
		m_pExprPosition->SymbolTable.add_variable("sz", m_vScale.z);

		m_pExprPosition->Expression.register_symbol_table(m_pExprPosition->SymbolTable);
		if (!m_pExprPosition->compileFormula())
			Logger::error("Draw Particle Morphing [{}]: Error while compiling the expression, default values used", identifier);

		// Build the particle array: InitPosition = source, Randomness = destination
		std::vector<ParticleMesh::Particle> Particles;
		Particles.resize(m_iNumParticles);

		for (int i = 0; i < m_iNumParticles; i++) {
			m_pExprPosition->executeFormula(); // Evaluate the expression on each particle
			Particles[i].Type = ParticleMesh::ParticleType::Shell;
			Particles[i].ID = (int32_t)i;
			Particles[i].InitPosition = srcSamples[i];	// Source position
			Particles[i].Randomness = dstSamples[i];	// Destination position (carried in Randomness)
			Particles[i].InitColor = glm::vec4(1, 1, 1, 1);
			Particles[i].Life = m_fDuration;
		}

		// Create the particle system
		m_pParticleMesh = new ParticleMesh();
		if (!m_pParticleMesh->init(Particles))
			return false;
		Particles.clear();

		// Create Shader variables
		m_pShader->use();
		m_pVars = new ShaderVars(this, m_pShader);

		// Read the shader variables
		for (auto& uni : shaderBlock[0]->uniform) {
			m_pVars->ReadString(uni);
		}

		// Validate and set shader variables
		m_pVars->validateAndSetValues();

		return !DEMO_checkGLError();
	}

	void sDrawParticleMorphing::init()
	{
	}

	void sDrawParticleMorphing::warmExec()
	{
		exec();
	}

	void sDrawParticleMorphing::exec()
	{
		// Start set render states and evaluating blending
		setRenderStatesStart();
		EvalBlendingStart();

		// Evaluate the expression
		m_pExprPosition->executeFormula();

		glm::mat4 projection = m_demo.m_cameraManager.getActiveProjection();
		glm::mat4 view = m_demo.m_cameraManager.getActiveView();

		// Build the model matrix (applied to both source and destination)
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_vTranslation);
		model = glm::rotate(model, glm::radians(m_vRotation.x), glm::vec3(1, 0, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.y), glm::vec3(0, 1, 0));
		model = glm::rotate(model, glm::radians(m_vRotation.z), glm::vec3(0, 0, 1));
		model = glm::scale(model, m_vScale);

		// Compute progress in [0, 1]
		float fProgress = 0.0f;
		if (m_fDuration > 0.0f)
			fProgress = glm::clamp(runTime / m_fDuration, 0.0f, 1.0f);

		// Get the shader
		m_pShader->use();
		m_pShader->setValue("m4ViewModel", view * model);
		m_pShader->setValue("m4Projection", projection);

		m_pShader->setValue("fProgress", fProgress);
		m_pShader->setValue("fDuration", m_fDuration);
		m_pShader->setValue("iNumParticles", m_iNumParticles);

		// Set the other shader variable values
		m_pVars->setValues();

		// Render particles
		m_pParticleMesh->render();

		// End evaluating blending and set render states back
		EvalBlendingEnd();
		setRenderStatesEnd();
	}

	void sDrawParticleMorphing::loadDebugStatic()
	{
		std::stringstream ss;
		ss << "Source model: " << (m_pModelSource ? m_pModelSource->filename : "nullptr") << std::endl;
		ss << " Vert. num: " << m_iModelSourceUniqueVerticesNum << std::endl;
		ss << "Destination model: " << (m_pModelDest ? m_pModelDest->filename : "nullptr") << std::endl;
		ss << " Vert. num: " << m_iModelDestUniqueVerticesNum << std::endl;
		ss << "Expression is: " << ((m_pExprPosition && m_pExprPosition->isValid()) ? "Valid" : "Faulty or Empty") << std::endl;
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		ss << "Duration (s): " << m_fDuration << std::endl;
		const float memUsedMb = m_pParticleMesh ? m_pParticleMesh->getMemUsedInMb() : 0.0f;
		ss << "Memory Used: " << std::fixed << std::setprecision(1) << memUsedMb << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticleMorphing::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}
