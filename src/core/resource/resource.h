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
	void Draw_Shdr_Basic();
	int tex_isaac2, tex_shotgan, tex_shotgan2, tex_merlucin, tex_xphere;
	int shdr_basic;
private:

	unsigned int obj_quad;
	string demoDir;
	Resource();

	void Load_Obj_Quad();
	void Load_Shdr_Basic();
	void Load_Tex_Spontz();

	static Resource* m_pThis;
};
#endif