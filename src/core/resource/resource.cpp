// resource.cpp
// Spontz Demogroup

#include "resource.h"
#include "main.h"

Resource& Resource::GetInstance() {
	static Resource r;
	return r;
}

void Resource::loadAllResources()
{
	LOG->Info(LogLevel::LOW, "Start Loading Engine Internal Resources");
	// Load Objects
	Load_Obj_QuadFullscreen();
	Load_Obj_Skybox();
	Load_Obj_Qube();
	// Load Shaders
	Load_Shaders();
	// Load Textures
	Load_Tex_Spontz();			// Spontz ridiculous pictures
	// Load Fonts --> This is no longer needed since we are using imGui for output text
	//Load_Text_Fonts();			// Text fonts
	// Load Lights
	Load_Lights();
}

Resource::Resource() {
	obj_quadFullscreen = obj_qube = obj_skybox = 0;
	shdr_ObjColor = shdr_QuadDepth = shdr_QuadTex = shdr_QuadTexPVM = shdr_QuadTexAlpha = shdr_QuadTexModel = shdr_QuadTexVFlipModel = shdr_Skybox = nullptr;
	tex_tv = 0;
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

	unsigned int quadVBO;
	glGenVertexArrays(1, &obj_quadFullscreen);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(obj_quadFullscreen);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
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
	// skybox VAO generation
	unsigned int skyboxVBO;
	glGenVertexArrays(1, &obj_skybox);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(obj_skybox);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
	// Qube VAO generation
	unsigned int qubeVBO;
	glGenVertexArrays(1, &obj_qube);	// VAO
	glGenBuffers(1, &qubeVBO);			// VBO
	
	glBindBuffer(GL_ARRAY_BUFFER, qubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(qubeVertices), &qubeVertices, GL_STATIC_DRAW);

	// Definition of Vertex Attributes
	glBindVertexArray(obj_qube);
	
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Texture Cords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Resource::Load_Shaders()
{
	shdr_QuadTex = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTex.glsl");
	shdr_QuadDepth = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadDepth.glsl");
	shdr_QuadTexAlpha = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexAlpha.glsl");
	shdr_QuadTexModel = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexModel.glsl");
	shdr_QuadTexPVM = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexPVM.glsl");
	shdr_QuadTexVFlipModel = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexVFlipModel.glsl");
	shdr_Skybox = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/skybox/skybox.glsl");
	shdr_ObjColor = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/ObjColor.glsl");
}

void Resource::Load_Tex_Spontz()
{
	tex_tv = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/tv.jpg");
}

void Resource::Load_Text_Fonts()
{
	// Since we are using imGui, fonts are no longer needed
	//DEMO->text = new Font(48, DEMO->dataFolder + "/resources/fonts/arial.ttf", DEMO->dataFolder + "/resources/shaders/font/font.glsl");
}

void Resource::Load_Lights()
{
	DEMO->lightManager.addLight(LightType::SpotLight);
	DEMO->lightManager.addLight(LightType::SpotLight);
	DEMO->lightManager.addLight(LightType::PointLight);
	DEMO->lightManager.addLight(LightType::PointLight);
}

// Draw a Quad with texture in full screen
void Resource::Draw_QuadFS(int textureNum)
{
	shdr_QuadTex->use();
	shdr_QuadTex->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();

	Draw_QuadFS();
}

// Draw a Quad with texture in full screen with alpha
void Resource::Draw_QuadFS(int textureNum, float alpha)
{
	shdr_QuadTexAlpha->use();
	shdr_QuadTexAlpha->setValue("alpha", alpha);
	shdr_QuadTexAlpha->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();
	
	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen
void Resource::Draw_QuadFBOFS(int fboNum, GLuint attachment)
{
	shdr_QuadTex->use();
	shdr_QuadTex->setValue("screenTexture", 0);
	DEMO->fboManager.bind_tex(fboNum, 0, attachment);
	
	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen
void Resource::Draw_QuadEfxFBOFS(int efxFboNum, GLuint attachment)
{
	shdr_QuadTex->use();
	shdr_QuadTex->setValue("screenTexture", 0);
	DEMO->efxBloomFbo.bind_tex(efxFboNum, 0, attachment);

	Draw_QuadFS();
}

// Draw a Quad in full screen. A texture can be specified and a model matrix
void Resource::Draw_Obj_QuadTex(int textureNum, glm::mat4 const* model)
{
	shdr_QuadTexModel->use();
	shdr_QuadTexModel->setValue("model", *model);
	shdr_QuadTexModel->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();

	Draw_QuadFS();
}

// Draw a Quad in full screen. A texture can be specified and the 3 matrix
void Resource::Draw_Obj_QuadTex(int textureNum, glm::mat4 *projection, glm::mat4* view, glm::mat4 *model)
{
	shdr_QuadTexPVM->use();
	shdr_QuadTexPVM->setValue("projection", *projection);
	shdr_QuadTexPVM->setValue("view", *view);
	shdr_QuadTexPVM->setValue("model", *model);
	shdr_QuadTexPVM->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();

	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen but no shader is called (needs a shader->use() call before)
void Resource::Draw_QuadFS()
{
	glBindVertexArray(obj_quadFullscreen);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


void Resource::Draw_Skybox(int cubemap)
{
	glBindVertexArray(obj_skybox);
	DEMO->textureManager.cubemap[cubemap]->bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void Resource::Draw_Cube()
{
	glBindVertexArray(obj_qube);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


