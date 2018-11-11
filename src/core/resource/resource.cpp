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
	Load_Obj_Quad();
	Load_Shdr_Basic();
	Load_Tex_Spontz();
}

Resource::Resource() {
	obj_quad = 0;
	shdr_basic = -1;
	demoDir = DEMO->demoDir;
}

void Resource::Load_Obj_Quad()
{
	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, EBO;
	glGenVertexArrays(1, &obj_quad);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(obj_quad);

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

void Resource::Load_Shdr_Basic()
{
	/////////////////////// BASIC: Textured shader
	shdr_basic = DEMO->shaderManager.addShader(demoDir + "/resources/shaders/basic/basic.vert", demoDir + "/resources/shaders/basic/basic.frag");
	if (shdr_basic != -1) {
		DEMO->shaderManager.shader[shdr_basic]->apply();
		DEMO->shaderManager.shader[shdr_basic]->setUniformMatrix4fv("world", GLDRV->world_matrix);
		DEMO->shaderManager.shader[shdr_basic]->setUniformMatrix3fv("normalMatrix", glm::inverse(glm::transpose(glm::mat3(GLDRV->world_matrix))));
		DEMO->shaderManager.shader[shdr_basic]->setUniformMatrix4fv("viewProj", GLDRV->projection_matrix * GLDRV->view_matrix);
		DEMO->shaderManager.shader[shdr_basic]->setUniform3fv("cam_pos", GLDRV->cam_position);
	}
	else
		LOG->Error("Could not load Basic shader!");

}

void Resource::Load_Tex_Spontz()
{
	tex_isaac2 = DEMO->textureManager.addTexture(demoDir + "/resources/textures/isaac2.jpg");
	tex_shotgan = DEMO->textureManager.addTexture(demoDir + "/resources/textures/shotgan.jpg");
	tex_shotgan2 = DEMO->textureManager.addTexture(demoDir + "/resources/textures/shotgan2.jpg");
	tex_merlucin = DEMO->textureManager.addTexture(demoDir + "/resources/textures/merlucin.jpg");
	tex_xphere = DEMO->textureManager.addTexture(demoDir + "/resources/textures/xphere.jpg");
}

// Draw a Quad in full screen. A texture and a Shader can be specified
void Resource::Draw_Obj_Quad(int texture_id, int shader_id)
{
	DEMO->textureManager.texture[texture_id]->bind();
	DEMO->shaderManager.shader[shader_id]->apply();
	glBindVertexArray(obj_quad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Draw a Quad in full screen. Spontz memeber picure used
void Resource::Draw_Obj_Quad()
{
	DEMO->textureManager.texture[tex_merlucin]->bind();
	Draw_Shdr_Basic();
	glBindVertexArray(obj_quad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// Draw a Quad in full screen. A texture can be specified. Textured shader will be applied
void Resource::Draw_Obj_Quad(int texture_id)
{
	DEMO->textureManager.texture[texture_id]->bind();
	Draw_Shdr_Basic();
	glBindVertexArray(obj_quad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Resource::Draw_Shdr_Basic()
{
	DEMO->shaderManager.shader[shdr_basic]->apply();
}


