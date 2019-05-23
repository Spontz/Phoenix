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
	Load_Obj_Quad_ColorTextured();
	Load_Obj_Quad_FBO();
	Load_Obj_Quad_FBO_Debug();
	Load_Shaders();
	Load_Tex_Spontz();
	Load_Text_Fonts();
}

Resource::Resource() {
	obj_quad_ColorText = 0;
	obj_quad_FBO = 0;
	shdr_basic = -1;
	demoDir = DEMO->demoDir;
}

void Resource::Load_Obj_Quad_ColorTextured()
{
	float vertices[] = {
		// positions	// colors	// texture coords
		 1,  1, 0,		1, 0, 0,	1, 0, // top right
		 1, -1, 0,		0, 1, 0,	1, 1, // bottom right
		-1, -1, 0,		0, 0, 1,	0, 1, // bottom left
		-1,  1, 0,		1, 1, 0,	0, 0  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, EBO;
	glGenVertexArrays(1, &obj_quad_ColorText);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(obj_quad_ColorText);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void Resource::Load_Obj_Quad_FBO()
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
	glGenVertexArrays(1, &obj_quad_FBO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(obj_quad_FBO);
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



void Resource::Load_Shaders()
{
	/////////////////////// BASIC: Textured shader
	shdr_basic = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/basic.vert", demoDir + "/resources/shaders/basic/basic.frag");
	if (shdr_basic != -1)
		DEMO->shaderManager.shader[shdr_basic]->use();
	else
		LOG->Error("Could not load Basic shader!");
	/////////////////////// BASIC: Shader to be used for FBO
	shdr_basicFBO = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/basicQuadFBO.vert", demoDir + "/resources/shaders/basic/basicQuadFBO.frag");
	if (shdr_basicFBO != -1)
		DEMO->shaderManager.shader[shdr_basicFBO]->use();
	else
		LOG->Error("Could not load Basic FBO shader!");

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

// Draw a Quad in full screen. A texture and a Shader can be specified
void Resource::Draw_Obj_Quad(int texture_id, int shader_id)
{
	DEMO->textureManager.texture[texture_id]->bind();
	DEMO->shaderManager.shader[shader_id]->use();
	glBindVertexArray(obj_quad_ColorText);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

// Draw a Quad in full screen
void Resource::Draw_Obj_Quad(glm::mat4 *model, glm::mat4 *view, glm::mat4 *projection, int tex_num)
{
	Shader *shad = DEMO->shaderManager.shader[shdr_basic];
	shad->use();
	shad->setValue("model", *model);
	shad->setValue("view", *view);
	shad->setValue("projection", *projection);
	shad->setValue("texture_diffuse1", 0);

	Texture *tex = DEMO->textureManager.texture[tex_num];
	tex->active();
	tex->bind();

	glBindVertexArray(obj_quad_ColorText);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

// Draw a Quad in full screen. A texture can be specified. Textured shader will be applied
void Resource::Draw_Obj_Quad(int texture_id)
{
	DEMO->textureManager.texture[texture_id]->bind();
	Draw_Shdr_Basic();
	glBindVertexArray(obj_quad_ColorText);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

// Draw a Quad with a FBO
void Resource::Draw_Obj_QuadFBO(int fbo_num)
{
	glBindVertexArray(obj_quad_FBO);
	DEMO->shaderManager.shader[RES->shdr_basicFBO]->use();
	DEMO->shaderManager.shader[RES->shdr_basicFBO]->setValue("screenTexture", 0);
	DEMO->fboManager.bind_tex(fbo_num);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// Draw the Debug Quads with a FBO
void Resource::Draw_Obj_QuadFBO_Debug(int quad, int fbo_num)
{
	glBindVertexArray(obj_quad_FBO_Debug[quad]);
	DEMO->shaderManager.shader[RES->shdr_basicFBO]->use();
	DEMO->shaderManager.shader[RES->shdr_basicFBO]->setValue("screenTexture", 0);
	DEMO->fboManager.bind_tex(fbo_num);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Resource::Draw_Obj_QuadTex(int shader_num, glm::mat4 *model, int tex_num)
{
	Shader *my_shad = DEMO->shaderManager.shader[shader_num];
	my_shad->use();
	my_shad->setValue("model", *model);

	glBindVertexArray(obj_quad_FBO);
	DEMO->textureManager.texture[tex_num]->active();
	DEMO->textureManager.texture[tex_num]->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Resource::Draw_Shdr_Basic()
{
	DEMO->shaderManager.shader[shdr_basic]->use();
}


