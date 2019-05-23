// resouce.h
// Spontz Demogroup

#ifndef RESOURCE_H
#define RESOURCE_H
#include <string>
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include "main.h"

#define NUM_FBO_DEBUG 4

using namespace std;
#define RES Resource::GetResource()

class Resource {
public:
	static Resource* GetResource();
	void loadAllResources();

	void Draw_Obj_Quad(int texture_id, int shader_id);
	void Draw_Obj_Quad(int texture_id);
	void Draw_Obj_Quad(glm::mat4 *model, glm::mat4 *view, glm::mat4 *projection, int tex_num);
	void Draw_Obj_QuadFBO(int fbo_num);
	void Draw_Obj_QuadFBO_Debug(int quad, int fbo_num);
	void Draw_Obj_QuadTex(int shader_num, glm::mat4 *model, int tex_num);
	void Draw_Shdr_Basic();
	int tex_tv, tex_isaac2, tex_shotgan, tex_shotgan2, tex_merlucin, tex_xphere;
	int shdr_basic, shdr_basicFBO;
private:

	unsigned int obj_quad_ColorText;
	unsigned int obj_quad_FBO;
	unsigned int obj_quad_FBO_Debug[NUM_FBO_DEBUG];
	string demoDir;
	Resource();

	void Load_Obj_Quad_ColorTextured();
	void Load_Obj_Quad_FBO();
	void Load_Obj_Quad_FBO_Debug();// Load debugging Quads for FBO
	void Load_Shaders();
	void Load_Tex_Spontz();
	void Load_Text_Fonts();

	static Resource* m_pThis;
};
#endif