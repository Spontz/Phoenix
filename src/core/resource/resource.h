// resouce.h
// Spontz Demogroup

#ifndef RESOURCE_H
#define RESOURCE_H
#include <string>

using namespace std;
#define RES Resource::GetResource()

class Resource {
public:
	static Resource* GetResource();
	void loadAllResources();

	void Draw_Obj_Quad(int texture_id, int shader_id);
	void Draw_Obj_Quad(int texture_id);
	void Draw_Obj_Quad();
	void Draw_Obj_QuadFBO(int fbo_num);
	void Draw_Shdr_Basic();
	int tex_tv, tex_isaac2, tex_shotgan, tex_shotgan2, tex_merlucin, tex_xphere;
	int shdr_basic, shdr_basicFBO;
private:

	unsigned int obj_quad_ColorText;
	unsigned int obj_quad_FBO;
	string demoDir;
	Resource();

	void Load_Obj_Quad_ColorTextured();
	void Load_Obj_Quad_FBO();
	void Load_Shaders();
	void Load_Tex_Spontz();
	void Load_Text_Fonts();

	static Resource* m_pThis;
};
#endif