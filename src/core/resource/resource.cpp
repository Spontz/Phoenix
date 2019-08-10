// resource.cpp
// Spontz Demogroup

#include "resource.h"
#include "main.h"

Resource* Resource::m_pThis = NULL;


Resource* Resource::GetResource() {
	if (m_pThis == NULL) {
		m_pThis = new Resource();
	}
	return m_pThis;
}

void Resource::loadAllResources()
{
	LOG->Info(LOG_LOW, "Start Loading Engine Internal Resources");
	// Load Objects
	Load_Obj_QuadFullscreen();
	Load_Obj_Quad_FBO_Debug();
	Load_Obj_Skybox();
	Load_Obj_Qube();
	// Load Shaders
	Load_Shaders();
	// Load Textures
	Load_Tex_Spontz();			// Spontz ridiculous pictures
	// Load Fonts
	Load_Text_Fonts();			// Text fonts
	// Load Lights
	Load_Lights();
	// Load effect resources
	Load_Bloom();
}

Resource::Resource() {
	obj_quadFullscreen = 0;
	shdr_QuadTex = -1;
	shdr_QuadTexModel = -1;
	bloomLoaded = false;
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

void Resource::Load_Obj_Quad_FBO_Debug()
{
	float inc = 2.0f / NUM_FBO_DEBUG; // Increment

	for (int i = 0; i < NUM_FBO_DEBUG; i++) {
		float quadVertices[] = {
			// positions   // texCoords
			 -1 + (inc*i),		-1+inc,	0, 1,
			 -1 + (inc*i),		-1,		0, 0,
			 -1 + inc + (inc*i),-1,		1, 0,

			 -1 + (inc*i),		-1+inc,	0, 1,
			 -1 + inc + (inc*i),-1,		1, 0,
			 -1 + inc + (inc*i),-1+inc,	1, 1
		};

		unsigned int quadVBO;
		glGenVertexArrays(1, &obj_quad_FBO_Debug[i]);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(obj_quad_FBO_Debug[i]);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}
	
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
	shdr_QuadTex = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTex.vert", DEMO->dataFolder + "/resources/shaders/basic/QuadTex.frag");
	shdr_QuadDepth = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadDepth.vert", DEMO->dataFolder + "/resources/shaders/basic/QuadDepth.frag");
	shdr_QuadTexAlpha = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexAlpha.vert", DEMO->dataFolder + "/resources/shaders/basic/QuadTexAlpha.frag");
	shdr_QuadTexModel = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexModel.vert", DEMO->dataFolder + "/resources/shaders/basic/QuadTexModel.frag");
	shdr_QuadTexVFlipModel = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/QuadTexVFlipModel.vert", DEMO->dataFolder + "/resources/shaders/basic/QuadTexVFlipModel.frag");
	shdr_Skybox = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/skybox/skybox.vert", DEMO->dataFolder + "/resources/shaders/skybox/skybox.frag");
	shdr_ObjColor = DEMO->shaderManager.addShader(DEMO->dataFolder + "/resources/shaders/basic/ObjColor.vert", DEMO->dataFolder + "/resources/shaders/basic/ObjColor.frag");
}

void Resource::Load_Tex_Spontz()
{
	tex_tv = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/tv.jpg");
	tex_isaac2 = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/isaac2.jpg");
	tex_shotgan = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/shotgan.jpg");
	tex_shotgan2 = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/shotgan2.jpg");
	tex_merlucin = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/merlucin.jpg");
	tex_xphere = DEMO->textureManager.addTexture(DEMO->dataFolder + "/resources/textures/xphere.jpg");
}

void Resource::Load_Text_Fonts()
{
	DEMO->text = new Font(48, DEMO->dataFolder + "/resources/fonts/arial.ttf", DEMO->dataFolder + "/resources/shaders/font/font.vert", DEMO->dataFolder + "/resources/shaders/font/font.frag");
}

void Resource::Load_Lights()
{
	DEMO->lightManager.addLight(LightType::SpotLight);
	DEMO->lightManager.addLight(LightType::SpotLight);
	DEMO->lightManager.addLight(LightType::PointLight);
	DEMO->lightManager.addLight(LightType::PointLight);
}

void Resource::Load_Bloom()
{
	if (this->bloomLoaded) {	// This means that Bloom has been loaded and we need to reload textures and FBO's
		glDeleteFramebuffers(2, this->bloomPingpongFBO);
		glDeleteTextures(2, this->bloomPingpongColorbuffer);
	}

	glGenFramebuffers(2, this->bloomPingpongFBO);
	glGenTextures(2, this->bloomPingpongColorbuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, this->bloomPingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, this->bloomPingpongColorbuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, GLDRV->width, GLDRV->height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->bloomPingpongColorbuffer[i], 0);
		// also check if framebuffers are complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LOG->Error("Resource::Load_Bloom: The internal framebuffers could not be created! The Bloom effects will not work properly!!");
			this->bloomLoaded = false;
			return;
		}
	}
	// Unbind texture and buffers
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	this->bloomLoaded = true;
}

// Draw a Quad with texture in full screen
void Resource::Draw_QuadFS(int textureNum)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTex];

	my_shad->use();
	my_shad->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();

	Draw_QuadFS();
}

// Draw a Quad with texture in full screen with alpha
void Resource::Draw_QuadFS(int textureNum, float alpha)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTexAlpha];

	my_shad->use();
	my_shad->setValue("alpha", alpha);
	my_shad->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();
	
	Draw_QuadFS();
}

// Draw a Quad with a FBO in full screen
void Resource::Draw_QuadFBOFS(int fboNum, GLuint attachment)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTex];

	my_shad->use();
	my_shad->setValue("screenTexture", 0);
	DEMO->fboManager.active();
	DEMO->fboManager.bind_tex(fboNum, attachment);
	
	Draw_QuadFS();
}

// Draw a Quad in full screen. A texture can be specified and a model matrix
void Resource::Draw_Obj_QuadTex(int textureNum, glm::mat4 *model)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTexModel];
	my_shad->use();
	my_shad->setValue("model", *model);
	my_shad->setValue("screenTexture", 0);
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

// Draw the Debug Quads with a FBO
void Resource::Draw_Obj_QuadFBO_Debug(int quad, int fbo_num, int fbo_attachment)
{
	int shader;
	if (DEMO->fboManager.fbo[fbo_num]->engineFormat == "DEPTH") // If we are drawing a DEPTH fbo, then we need to use another shader
		shader = RES->shdr_QuadDepth;
	else
		shader = RES->shdr_QuadTex;

	DEMO->shaderManager.shader[shader]->use();
	DEMO->shaderManager.shader[shader]->setValue("screenTexture", 0);
	DEMO->fboManager.active();
	DEMO->fboManager.bind_tex(fbo_num, fbo_attachment);
	glBindVertexArray(obj_quad_FBO_Debug[quad]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
}


