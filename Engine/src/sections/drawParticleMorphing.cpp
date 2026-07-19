#include "main.h"
#include "core/renderer/ParticleMesh.h"
#include "core/drivers/mathdriver.h"
#include "core/renderer/ShaderVars.h"

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
		SP_Model m_pModelSource;
		SP_Model m_pModelDest;

		// Particle engine variables
		int				m_iNumParticles = 0;
		float			m_fDuration = 0;
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

	// Deterministic uniform sampling of a vertex list into exactly 'count' points.
	// - If count <= source.size(): pick a uniform subset using a deterministic stride (no duplicates).
	// - If count >  source.size(): interpolate linearly between consecutive vertices to fill the gap.
	static std::vector<glm::vec3> sampleUniform(const std::vector<glm::vec3>& source, size_t count)
	{
		std::vector<glm::vec3> out;
		out.reserve(count);
		const size_t srcSize = source.size();
		if (srcSize == 0 || count == 0)
			return out;

		if (count <= srcSize) {
			// Uniform subset: deterministic stride so points are evenly spread
			for (size_t i = 0; i < count; i++) {
				// Map i in [0, count) to a vertex index in [0, srcSize)
				size_t idx = (i * srcSize) / count;
				if (idx >= srcSize) idx = srcSize - 1;
				out.push_back(source[idx]);
			}
		}
		else {
			// Interpolate to generate more points than available vertices
			for (size_t i = 0; i < count; i++) {
				float t = (float)i / (float)(count - 1); // [0, 1]
				float fpos = t * (float)(srcSize - 1);
				size_t a = (size_t)fpos;
				if (a >= srcSize - 1) a = srcSize - 2;
				float frac = fpos - (float)a;
				glm::vec3 p = glm::mix(source[a], source[a + 1], frac);
				out.push_back(p);
			}
		}
		return out;
	}

	bool sDrawParticleMorphing::load()
	{
		// script validation:
		// param[0] = number of particles
		// param[1] = duration (in seconds) of the morphing
		// strings[0] = source scene model
		// strings[1] = destination scene model
		// shaderBlock[0] = shader
		// expressionRun = positioning expression (tx,ty,tz,rx,ry,rz,sx,sy,sz)
		if ((param.size() != 2) || (strings.size() != 2) || (shaderBlock.size() != 1)) {
			Logger::error("Draw Particle Morphing [{}]: 2 params (NumParticles & Duration), 2 strings (source & dest scene), 1 shader and 1 expression needed", identifier);
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
		if (m_fDuration <= 0) {
			Logger::error("Draw Particle Morphing [{}]: Duration must be greater than 0", identifier);
			return false;
		}

		// Load the shader
		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		// Load source and destination models
		m_pModelSource = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[0]);
		m_pModelDest = m_demo.m_modelManager.addModel(m_demo.m_dataFolder + strings[1]);
		if (!m_pModelSource || !m_pModelDest)
			return false;

		// Load unique vertices for both models
		m_pModelSource->loadUniqueVertices();
		m_pModelDest->loadUniqueVertices();

		// Collect all unique vertices from both scenes
		std::vector<glm::vec3> srcVerts = collectUniqueVertices(m_pModelSource);
		std::vector<glm::vec3> dstVerts = collectUniqueVertices(m_pModelDest);

		if (srcVerts.empty()) {
			Logger::error("Draw Particle Morphing [{}]: No vertices found in the source model", identifier);
			return false;
		}
		if (dstVerts.empty()) {
			Logger::error("Draw Particle Morphing [{}]: No vertices found in the destination model", identifier);
			return false;
		}

		// Sample uniformly and independently on both source and destination
		std::vector<glm::vec3> srcSamples = sampleUniform(srcVerts, (size_t)m_iNumParticles);
		std::vector<glm::vec3> dstSamples = sampleUniform(dstVerts, (size_t)m_iNumParticles);

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
		ss << "Destination model: " << (m_pModelDest ? m_pModelDest->filename : "nullptr") << std::endl;
		ss << "Expression is: " << (m_pExprPosition->isValid() ? "Valid" : "Faulty or Empty") << std::endl;
		ss << "Num Particles: " << m_iNumParticles << std::endl;
		ss << "Duration: " << m_fDuration << std::endl;
		ss << "Memory Used: " << std::format("{:.1f}", m_pParticleMesh->getMemUsedInMb()) << " Mb" << std::endl;
		debugStatic = ss.str();
	}

	std::string sDrawParticleMorphing::debug()
	{
		std::stringstream ss;
		ss << debugStatic;
		return ss.str();
	}
}
