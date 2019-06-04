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
	// Load Shaders
	Load_Shaders();
	// Load Textures
	Load_Tex_Spontz();			// Spontz ridiculous pictures
	// Load Fonts
	Load_Text_Fonts();			// Text fonts
}

Resource::Resource() {
	obj_quadFullscreen = 0;
	shdr_QuadTex = -1;
	shdr_QuadTexModel = -1;
	demoDir = DEMO->demoDir;
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

void Resource::Load_Shaders()
{
	shdr_QuadTex = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/QuadTex.vert", demoDir + "/resources/shaders/basic/QuadTex.frag");
	shdr_QuadDepth = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/QuadDepth.vert", demoDir + "/resources/shaders/basic/QuadDepth.frag");
	shdr_QuadTexAlpha = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/QuadTexAlpha.vert", demoDir + "/resources/shaders/basic/QuadTexAlpha.frag");
	shdr_QuadTexModel = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/QuadTexModel.vert", demoDir + "/resources/shaders/basic/QuadTexModel.frag");
	shdr_Skybox = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/skybox/skybox.vert", demoDir + "/resources/shaders/skybox/skybox.frag");
}

void Resource::Load_Tex_Spontz()
{
	tex_tv = DEMO->textureManager.addTexture(demoDir + "/resources/textures/tv.jpg");
	tex_isaac2 = DEMO->textureManager.addTexture(demoDir + "/resources/textures/isaac2.jpg");
	tex_shotgan = DEMO->textureManager.addTexture(demoDir + "/resources/textures/shotgan.jpg");
	tex_shotgan2 = DEMO->textureManager.addTexture(demoDir + "/resources/textures/shotgan2.jpg");
	tex_merlucin = DEMO->textureManager.addTexture(demoDir + "/resources/textures/merlucin.jpg");
	tex_xphere = DEMO->textureManager.addTexture(demoDir + "/resources/textures/xphere.jpg");
}

void Resource::Load_Text_Fonts()
{
	DEMO->text = new Font(48, demoDir + "/resources/fonts/arial.ttf", demoDir + "/resources/shaders/font/font.vert", demoDir + "/resources/shaders/font/font.frag");
}


// Draw a Quad with texture in full screen
void Resource::Draw_QuadFS(int textureNum)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTex];

	glBindVertexArray(obj_quadFullscreen);
	my_shad->use();
	my_shad->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// Draw a Quad with texture in full screen with alpha
void Resource::Draw_QuadFS(int textureNum, float alpha)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTexAlpha];

	glBindVertexArray(obj_quadFullscreen);
	my_shad->use();
	my_shad->setValue("alpha", alpha);
	my_shad->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// Draw a Quad with a FBO in full screen
void Resource::Draw_QuadFBOFS(int fboNum)
{
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTex];

	glBindVertexArray(obj_quadFullscreen);
	my_shad->use();
	my_shad->setValue("screenTexture", 0);
	DEMO->fboManager.bind_tex(fboNum);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// Draw a Quad with a FBO in full screen but no shader is called (needs a shader->use() call before)
void Resource::Draw_QuadFS()
{
	glBindVertexArray(obj_quadFullscreen);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


// Draw a Quad in full screen. A texture can be specified and a model matrix
void Resource::Draw_Obj_QuadTex(int textureNum, glm::mat4 *model)
{
	glBindVertexArray(obj_quadFullscreen);
	Shader *my_shad = DEMO->shaderManager.shader[shdr_QuadTexModel];
	my_shad->use();
	my_shad->setValue("model", *model);
	my_shad->setValue("screenTexture", 0);
	DEMO->textureManager.texture[textureNum]->active();
	DEMO->textureManager.texture[textureNum]->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


void Resource::Draw_Skybox(int cubemap)
{
	glBindVertexArray(obj_skybox);
	DEMO->textureManager.cubemap[cubemap]->active();
	DEMO->textureManager.cubemap[cubemap]->bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// Draw the Debug Quads with a FBO
void Resource::Draw_Obj_QuadFBO_Debug(int quad, int fbo_num)
{
	int shader;
	if (DEMO->fboManager.fbo[fbo_num]->engineFormat == "DEPTH") // If we are drawing a DEPTH fbo, then we need to use another shader
		shader = RES->shdr_QuadDepth;
	else
		shader = RES->shdr_QuadTex;

	glBindVertexArray(obj_quad_FBO_Debug[quad]);
	DEMO->shaderManager.shader[shader]->use();
	DEMO->shaderManager.shader[shader]->setValue("screenTexture", 0);
	DEMO->fboManager.active(0);
	DEMO->fboManager.bind_tex(fbo_num);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


