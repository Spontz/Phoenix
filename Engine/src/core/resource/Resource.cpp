// Resource.cpp
// Spontz Demogroup

#include "main.h"

#include "resource.h"

namespace Phoenix {

	void Resource::loadAllResources()
	{
		Logger::info(LogLevel::low, "Start loading engine internal resources");
		// Load Objects
		loadObjQuadFullscreen();
		loadObjSkybox();
		loadObjCube();
		// Load grid
		loadGrid();
		// Load Shaders
		loadShaders();
		// Load Textures
		loadTexSpontz();
		// Load Lights
		loadLights();
		Logger::info(LogLevel::low, "End loading engine internal resources");
	}

	void Resource::unloadAllResources()
	{
		// Vertex Array Objects
		m_spQuadFullScreen.reset();
		m_spSkybox.reset();
		m_spCube.reset();
		m_spGrid.reset();

		// Textures
		m_spTVImage.reset();

		// Shaders
		m_spShdrQuadTex.reset();
		m_spShdrQuadDepth.reset();
		m_spShdrQuadTexAlpha.reset();
		m_spShdrQuadTexModel.reset();
		m_spShdrQuadTexPVM.reset();
		m_spShdrQuadTexVFlipModel.reset();
		m_spShdrSkybox.reset();
		m_spShdrObjColor.reset();
		m_spShdrGrid.reset();
	}

	Resource::Resource()
		:
		m_demo(*DEMO),
		m_gridSize(10.0f),
		m_gridSlices(10)
	{
	}

	Resource::~Resource()
	{
		unloadAllResources();
	}


	void Resource::loadObjQuadFullscreen()
	{
		static constexpr float quadVertices[] = {
			// positions   // texCoords
			 -1,  1,  0, 1,
			 -1, -1,  0, 0,
			  1, -1,  1, 0,

			 -1,  1,  0, 1,
			  1, -1,  1, 0,
			  1,  1,  1, 1
		};

		// Creatr the Vertex Array
		m_spQuadFullScreen = std::make_shared<VertexArray>();

		// Create & Load the Vertex Buffer
		auto spVB = std::make_shared<VertexBuffer>(&quadVertices, static_cast<uint32_t>(sizeof(quadVertices)));
		spVB->SetLayout({
			{ ShaderDataType::Float2,	"aPos"},
			{ ShaderDataType::Float2,	"aTexCoords"},
			});

		m_spQuadFullScreen->AddVertexBuffer(spVB);

		// Create & Load the Index Buffer :: Not really needed since the vertice are already sorted
		//uint32_t quadIndices[] = { 0,1,2,3,4,5 };
		//IndexBuffer* ib = new IndexBuffer(&quadIndices[0], 6);
		//m_pQuadFullScreen->SetIndexBuffer(ib);

		m_spQuadFullScreen->unbind();
	}

	void Resource::loadObjSkybox()
	{
		static constexpr float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		// Creatr the Vertex Array
		m_spSkybox = std::make_shared<VertexArray>();

		// Create & Load the Vertex Buffer
		auto spVB = std::make_shared<VertexBuffer>(&skyboxVertices, static_cast<uint32_t>(sizeof(skyboxVertices)));
		spVB->SetLayout({
			{ ShaderDataType::Float3,	"aPos"},
			});

		m_spSkybox->AddVertexBuffer(spVB);
		m_spSkybox->unbind();
	}

	void Resource::loadObjCube()
	{
		static constexpr float qubeVertices[] = {
			// positions          // normals           // texture coords
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
		};

		// Creatr the Vertex Array
		m_spCube = std::make_shared<VertexArray>();

		// Create & Load the Vertex Buffer
		auto spVB = std::make_shared<VertexBuffer>(&qubeVertices, static_cast<uint32_t>(sizeof(qubeVertices)));
		spVB->SetLayout({
			{ ShaderDataType::Float3,	"aPos"},
			{ ShaderDataType::Float3,	"aNormal"},
			{ ShaderDataType::Float2,	"aTexCoords"},
			});

		m_spCube->AddVertexBuffer(spVB);
		m_spCube->unbind();
	}

	void Resource::loadShaders()
	{
		m_spShdrQuadTex = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTex.glsl");
		m_spShdrQuadDepth = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadDepth.glsl");
		m_spShdrQuadTexAlpha = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexAlpha.glsl");
		m_spShdrQuadTexModel = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexModel.glsl");
		m_spShdrQuadTexPVM = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexPVM.glsl");
		m_spShdrQuadTexVFlipModel = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexVFlipModel.glsl");
		m_spShdrSkybox = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/skybox/skybox.glsl");
		m_spShdrObjColor = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/ObjColor.glsl");
		m_spShdrGrid = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/Grid.glsl");
	}

	void Resource::loadTexSpontz()
	{
		m_spTVImage = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/textures/tv.jpg");
	}

	void Resource::loadLights()
	{
		m_demo.m_lightManager.addLight(LightType::SpotLight);
		m_demo.m_lightManager.addLight(LightType::SpotLight);
		m_demo.m_lightManager.addLight(LightType::PointLight);
		m_demo.m_lightManager.addLight(LightType::PointLight);
	}

	void Resource::loadGrid()
	{
		std::vector<glm::vec3> vertices;
		std::vector<uint32_t> indices;

		float start = 0.0f;
		float step = 1.0f / static_cast<float>(m_gridSlices);

		for (int j = 0; j <= m_gridSlices; ++j) {
			for (int i = 0; i <= m_gridSlices; ++i) {
				float x = m_gridSize * (start + static_cast<float>(i) * step);
				float y = 0;
				float z = m_gridSize * (start + static_cast<float>(j) * step);
				vertices.push_back(glm::vec3(x, y, z));
			}
		}

		for (int j = 0; j < m_gridSlices; ++j) {
			for (int i = 0; i < m_gridSlices; ++i) {

				int row1 = j * (m_gridSlices + 1);
				int row2 = (j + 1) * (m_gridSlices + 1);

				indices.push_back(row1 + i);
				indices.push_back(row1 + i + 1);
				indices.push_back(row1 + i + 1);
				indices.push_back(row2 + i + 1);

				indices.push_back(row2 + i + 1);
				indices.push_back(row2 + i);
				indices.push_back(row2 + i);
				indices.push_back(row1 + i);

			}
		}

		// Creatr the Vertex Array
		m_spGrid = std::make_shared<VertexArray>();

		// Create & Load the Vertex Buffer
		auto spVB = std::make_shared<VertexBuffer>(
			&vertices[0],
			static_cast<uint32_t>(sizeof(glm::vec3)) * static_cast<uint32_t>(vertices.size())
			);

		spVB->SetLayout({ {ShaderDataType::Float3, "aPos"} });
		m_spGrid->AddVertexBuffer(spVB);

		// Create & Load the Index Buffer :: Not really needed since the vertice are already sorted
		auto spIB = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
		m_spGrid->SetIndexBuffer(spIB);

		m_spGrid->unbind();
	}

	// Draw a Quad with texture in full screen with alpha
	void Resource::drawQuadFS(SP_Texture spTexture, float alpha)
	{
		m_spShdrQuadTexAlpha->use();
		m_spShdrQuadTexAlpha->setValue("alpha", alpha);
		m_spShdrQuadTexAlpha->setValue("screenTexture", 0);
		spTexture->bind();

		drawQuadFS();
	}

	// Draw a Quad with a FBO in full screen
	void Resource::drawQuadFboFS(Fbo* fbo, GLuint attachment)
	{
		m_spShdrQuadTex->use();
		m_spShdrQuadTex->setValue("screenTexture", 0);
		fbo->bind_tex(0, attachment);

		drawQuadFS();
	}

	// Draw a Quad with a FBO in full screen
	void Resource::drawQuadEfxFboFS(int32_t efxFboNum, GLuint attachment)
	{
		m_spShdrQuadTex->use();
		m_spShdrQuadTex->setValue("screenTexture", 0);
		m_demo.m_efxBloomFbo.bind_tex(efxFboNum, 0, attachment);

		drawQuadFS();
	}

	// Draw a Quad in full screen. A texture can be specified and a model matrix
	void Resource::drawObjQuadTex(SP_Texture tex, glm::mat4 const* model)
	{
		m_spShdrQuadTexModel->use();
		m_spShdrQuadTexModel->setValue("model", *model);
		m_spShdrQuadTexModel->setValue("screenTexture", 0);
		tex->bind();

		drawQuadFS();
	}

	// Draw a Quad in full screen. A texture can be specified and the 3 matrix
	void Resource::drawObjQuadTex(
		SP_Texture spTex,
		glm::mat4 const* pProj,
		glm::mat4 const* pView,
		glm::mat4 const* pWorld
	)
	{
		m_spShdrQuadTexPVM->use();
		m_spShdrQuadTexPVM->setValue("projection", *pProj);
		m_spShdrQuadTexPVM->setValue("view", *pView);
		m_spShdrQuadTexPVM->setValue("model", *pWorld);
		m_spShdrQuadTexPVM->setValue("screenTexture", 0);
		spTex->bind();

		drawQuadFS();
	}

	// Draw a Quad with a FBO in full screen but no shader is called (needs a shader->use() call
	// before)
	void Resource::drawQuadFS()
	{
		m_spQuadFullScreen->bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);// m_pQuadFullScreen->GetIndexBuffer()->GetCount());
		m_spQuadFullScreen->unbind();
	}

	void Resource::drawSkybox(SP_Cubemap cubemap)
	{
		m_spSkybox->bind();
		cubemap->bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		m_spSkybox->unbind();
	}

	void Resource::drawCube()
	{
		m_spCube->bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		m_spCube->unbind();
	}

	void Resource::drawOneGrid(glm::vec3 const& color, glm::mat4 const* MVP)
	{
		if (!m_spShdrGrid)
			return;

		glEnable(GL_DEPTH_TEST);
		m_spShdrGrid->use();
		m_spShdrGrid->setValue("MVP", *MVP);
		m_spShdrGrid->setValue("color", color);

		m_spGrid->bind();

		glDrawElements(GL_LINES, m_spGrid->getIndexBuffer()->GetCount(), GL_UNSIGNED_INT, NULL);
		m_spGrid->unbind();
	}

	void Resource::draw3DGrid(bool drawAxisX, bool drawAxisY, bool drawAxisZ)
	{
		glm::mat4 MVP;
		glm::mat4 VP;

		glm::mat4 projection = m_demo.m_cameraManager.getActiveProjection();
		glm::mat4 view = m_demo.m_cameraManager.getActiveView();

		VP = projection * view;

		glm::mat4 model = glm::mat4(1.0f);

		// X Axis
		if (drawAxisX)
		{
			MVP = VP * model;
			drawOneGrid(glm::vec3(1, 0, 0), &MVP);
		}

		// Y Axis
		if (drawAxisY)
		{
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
			MVP = VP * model;
			drawOneGrid(glm::vec3(0, 1, 0), &MVP);
		}

		// Z Axis
		if (drawAxisZ)
		{
			model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
			MVP = VP * model;
			drawOneGrid(glm::vec3(0, 0, 1), &MVP);
		}
	}

}
