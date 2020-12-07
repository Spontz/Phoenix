// Resource.cpp
// Spontz Demogroup

#include "main.h"

#include "resource.h"

void Resource::loadAllResources()
{
	Logger::info(LogLevel::low, "Start loading engine internal resources");
	// Load Objects
	Load_Obj_QuadFullscreen();
	Load_Obj_Skybox();
	Load_Obj_Qube();
	// Load grid
	Load_Grid();
	// Load Shaders
	Load_Shaders();
	// Load Textures
	Load_Tex_Spontz();
	// Load Fonts --> This is no longer needed since we are using imGui for output text
	//Load_Text_Fonts();			// Text fonts
	// Load Lights
	Load_Lights();
	Logger::info(LogLevel::low, "End loading engine internal resources");
}

Resource::Resource()
	:
	m_demo(demokernel::GetInstance()),
	m_pQuadFullScreen(nullptr),
	m_pSkybox(nullptr),
	m_pQube(nullptr),
	m_pTVImage(nullptr),
	m_pGrid(nullptr),
	m_gridSize(1.0f),
	m_gridSlices(10)
{
	m_pShdrObjColor = m_pShdrQuadDepth = m_pShdrQuadTex = m_pShdrQuadTexPVM = m_pShdrQuadTexAlpha = m_pShdrQuadTexModel = m_pShdrQuadTexVFlipModel = m_pShdrSkybox = m_pShdrGrid = nullptr;
}

Resource::~Resource()
{
	delete m_pQuadFullScreen;
	delete m_pSkybox;
	delete m_pQube;
}


void Resource::Load_Obj_QuadFullscreen()
{
	float quadVertices[] = {
	// positions   // texCoords
	 -1,  1,  0, 1,
	 -1, -1,  0, 0,
	  1, -1,  1, 0,

	 -1,  1,  0, 1,
	  1, -1,  1, 0,
	  1,  1,  1, 1
	};

	// Creatr the Vertex Array
	m_pQuadFullScreen = new VertexArray();

	// Create & Load the Vertex Buffer
	VertexBuffer* vb = new VertexBuffer(&quadVertices, static_cast<uint32_t>(sizeof(quadVertices)));
	vb->SetLayout({
		{ ShaderDataType::Float2,	"aPos"},
		{ ShaderDataType::Float2,	"aTexCoords"},
		});

	m_pQuadFullScreen->AddVertexBuffer(vb);

	// Create & Load the Index Buffer :: Not really needed since the vertice are already sorted
	//uint32_t quadIndices[] = { 0,1,2,3,4,5 };
	//IndexBuffer* ib = new IndexBuffer(&quadIndices[0], 6);
	//m_pQuadFullScreen->SetIndexBuffer(ib);

	m_pQuadFullScreen->Unbind();
}

void Resource::Load_Obj_Skybox()
{
	float skyboxVertices[] = {
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
	m_pSkybox = new VertexArray();

	// Create & Load the Vertex Buffer
	VertexBuffer* vb = new VertexBuffer(&skyboxVertices, static_cast<uint32_t>(sizeof(skyboxVertices)));
	vb->SetLayout({
		{ ShaderDataType::Float3,	"aPos"},
		});

	m_pSkybox->AddVertexBuffer(vb);
	m_pSkybox->Unbind();
}

void Resource::Load_Obj_Qube()
{
	float qubeVertices[] = {
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
	m_pQube = new VertexArray();

	// Create & Load the Vertex Buffer
	VertexBuffer* vb = new VertexBuffer(&qubeVertices, static_cast<uint32_t>(sizeof(qubeVertices)));
	vb->SetLayout({
		{ ShaderDataType::Float3,	"aPos"},
		{ ShaderDataType::Float3,	"aNormal"},
		{ ShaderDataType::Float2,	"aTexCoords"},
		});

	m_pQube->AddVertexBuffer(vb);
	m_pQube->Unbind();
}

void Resource::Load_Shaders()
{
	m_pShdrQuadTex = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTex.glsl");
	m_pShdrQuadDepth = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadDepth.glsl");
	m_pShdrQuadTexAlpha = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexAlpha.glsl");
	m_pShdrQuadTexModel = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexModel.glsl");
	m_pShdrQuadTexPVM = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexPVM.glsl");
	m_pShdrQuadTexVFlipModel = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/QuadTexVFlipModel.glsl");
	m_pShdrSkybox = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/skybox/skybox.glsl");
	m_pShdrObjColor = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/ObjColor.glsl");
	m_pShdrGrid = m_demo.m_shaderManager.addShader(m_demo.m_dataFolder + "/resources/shaders/basic/Grid.glsl");
}

void Resource::Load_Tex_Spontz()
{
	m_pTVImage = m_demo.m_textureManager.addTexture(m_demo.m_dataFolder + "/resources/textures/tv.jpg");
}

void Resource::Load_Text_Fonts()
{
	// Since we are using imGui, fonts are no longer needed
	//m_demo.text = new Font(48, m_demo.dataFolder + "/resources/fonts/arial.ttf", m_demo.dataFolder + "/resources/shaders/font/font.glsl");
}

void Resource::Load_Lights()
{
	m_demo.m_lightManager.addLight(LightType::SpotLight);
	m_demo.m_lightManager.addLight(LightType::SpotLight);
	m_demo.m_lightManager.addLight(LightType::PointLight);
	m_demo.m_lightManager.addLight(LightType::PointLight);
}

void Resource::Load_Grid()
{
	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;

	for (int j = 0; j <= m_gridSlices; ++j) {
		for (int i = 0; i <= m_gridSlices; ++i) {
			float x = m_gridSize * ((float)i / (float)m_gridSlices);
			float y = 0;
			float z = m_gridSize * ((float)j / (float)m_gridSlices);
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

	if (m_pGrid)
		delete m_pGrid;

	// Creatr the Vertex Array
	m_pGrid = new VertexArray();

	// Create & Load the Vertex Buffer
	VertexBuffer* vb = new VertexBuffer(&vertices[0], static_cast<uint32_t>(sizeof(glm::vec3)*vertices.size()));
	vb->SetLayout({
		{ ShaderDataType::Float3,	"aPos"},
		});

	m_pGrid->AddVertexBuffer(vb);

	// Create & Load the Index Buffer :: Not really needed since the vertice are already sorted
	IndexBuffer* ib = new IndexBuffer(&indices[0], static_cast<uint32_t>(indices.size()));
	m_pGrid->SetIndexBuffer(ib);

	m_pGrid->Unbind();
}

// Draw a Quad with texture in full screen with alpha
void Resource::Draw_QuadFS(Texture* tex, float alpha)
{
	m_pShdrQuadTexAlpha->use();
	m_pShdrQuadTexAlpha->setValue("alpha", alpha);
	m_pShdrQuadTexAlpha->setValue("screenTexture", 0);
	tex->bind();

	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen
void Resource::Draw_QuadFBOFS(Fbo* fbo, GLuint attachment)
{
	m_pShdrQuadTex->use();
	m_pShdrQuadTex->setValue("screenTexture", 0);
	fbo->bind_tex(0, attachment);

	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen
void Resource::Draw_QuadEfxFBOFS(int efxFboNum, GLuint attachment)
{
	m_pShdrQuadTex->use();
	m_pShdrQuadTex->setValue("screenTexture", 0);
	m_demo.m_efxBloomFbo.bind_tex(efxFboNum, 0, attachment);

	Draw_QuadFS();
}

// Draw a Quad in full screen. A texture can be specified and a model matrix
void Resource::Draw_Obj_QuadTex(Texture* tex, glm::mat4 const* model)
{
	m_pShdrQuadTexModel->use();
	m_pShdrQuadTexModel->setValue("model", *model);
	m_pShdrQuadTexModel->setValue("screenTexture", 0);
	tex->bind();

	Draw_QuadFS();
}

// Draw a Quad in full screen. A texture can be specified and the 3 matrix
void Resource::Draw_Obj_QuadTex(Texture* tex, glm::mat4 const* projection, glm::mat4 const* view, glm::mat4 const* model)
{
	m_pShdrQuadTexPVM->use();
	m_pShdrQuadTexPVM->setValue("projection", *projection);
	m_pShdrQuadTexPVM->setValue("view", *view);
	m_pShdrQuadTexPVM->setValue("model", *model);
	m_pShdrQuadTexPVM->setValue("screenTexture", 0);
	tex->bind();

	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen but no shader is called (needs a shader->use() call before)
void Resource::Draw_QuadFS()
{
	m_pQuadFullScreen->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);// m_pQuadFullScreen->GetIndexBuffer()->GetCount());
	m_pQuadFullScreen->Unbind();
}


void Resource::Draw_Skybox(Cubemap* cubemap)
{
	m_pSkybox->Bind();
	cubemap->bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	m_pSkybox->Unbind();
}

void Resource::Draw_Cube()
{
	m_pQube->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	m_pQube->Unbind();
}

void Resource::Draw_Grid(glm::vec3 const color, glm::mat4 const* MVP)
{
	if (!m_pShdrGrid)
		return;
	glEnable(GL_DEPTH_TEST);
	m_pShdrGrid->use();
	m_pShdrGrid->setValue("MVP", *MVP);
	m_pShdrGrid->setValue("color", color);

	m_pGrid->Bind();
	
	glDrawElements(GL_LINES, m_pGrid->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, NULL);
	m_pGrid->Unbind();
}


