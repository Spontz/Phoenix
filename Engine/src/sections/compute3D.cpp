#include "main.h"
#include "core/renderer/ShaderVars.h"

namespace Phoenix {
	class sCompute3D final : public Section {
	public:
		sCompute3D();
		~sCompute3D();

	public:
		bool		load();
		void		init();
		void		exec();
		std::string debug();

	private:
		void executeCompute();

		SP_Shader	m_pShader;
		ShaderVars* m_pVars = nullptr;

		int		m_volumeIndex = 0;
		bool	m_regenerateEveryFrame = true;
		bool	m_executed = false;

		glm::uvec3 m_localSize = glm::uvec3(1);
	};


	// ******************************************************************

	Section* instance_compute3D() {
		return new sCompute3D();
	}

	sCompute3D::sCompute3D() {
		type = SectionType::Compute3D;
	}

	sCompute3D::~sCompute3D() {
		delete m_pVars;
	}

	bool sCompute3D::load() {

		if (param.size() != 2) {
			Logger::error("Compute3D [{}]: 2 params required: volume index, regenerate every frame", identifier);
			return false;
		}

		if (shaderBlock.size() != 1) {
			Logger::error("Compute3D [{}]: 1 shader is required", identifier);
			return false;
		}

		m_volumeIndex = static_cast<int>(param[0]);
		m_regenerateEveryFrame = static_cast<bool>(param[1]);

		if (m_volumeIndex < 0) {
			Logger::error("Compute3D [{}]: invalid volume index {}", identifier, m_volumeIndex);
			return false;
		}

		m_pShader = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + shaderBlock[0]->filename);
		if (!m_pShader)
			return false;

		if (!m_pShader->isCompute()) {
			Logger::error("Compute3D [{}]: shader '{}' is not a compute shader", identifier, shaderBlock[0]->filename);
			return false;
		}

		// Check if the volume exists
		auto volume = m_demo.m_textureManager.getRuntime3D( m_volumeIndex);
		if (!volume)
			return false;

		m_pShader->use();

		m_pVars = new ShaderVars(this, m_pShader);

		for (auto& uni : shaderBlock[0]->uniform)
			m_pVars->ReadString(uni);

		m_pVars->validateAndSetValues();

		m_localSize = m_pShader->getComputeWorkGroupSize();
		if (m_localSize.x == 0 || m_localSize.y == 0 ||	m_localSize.z == 0) {
			Logger::error("Compute3D [{}]: invalid compute workgroup size", identifier);
			return false;
		}

		return !DEMO_checkGLError();
	}

	void sCompute3D::init() {
		m_executed = false;

		if (!m_regenerateEveryFrame)
			executeCompute();
	}

	void sCompute3D::exec() {

		if (!m_regenerateEveryFrame)
			return;

		executeCompute();

	}

	void sCompute3D::executeCompute()
	{
		auto volume = m_demo.m_textureManager.getRuntime3D(m_volumeIndex);

		if (!volume || !m_pShader)
			return;

		const uint32_t groupsX = (static_cast<uint32_t>(volume->getWidth())  + m_localSize.x - 1) / m_localSize.x;
		const uint32_t groupsY = (static_cast<uint32_t>(volume->getHeight()) + m_localSize.y - 1) / m_localSize.y;
		const uint32_t groupsZ = (static_cast<uint32_t>(volume->getDepth())  + m_localSize.z - 1) / m_localSize.z;

		m_pShader->use();

		m_pVars->setValues();

		volume->bindImage(0, GL_WRITE_ONLY);

		m_pShader->dispatch(groupsX, groupsY, groupsZ);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

		m_executed = true;
	}

	std::string sCompute3D::debug() {
		std::stringstream ss;
		ss << "Volume=" << m_volumeIndex;
		return ss.str();
	}
}