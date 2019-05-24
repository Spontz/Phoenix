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

	void Draw_QuadFS(int textureNum);	// Draws a quad with fullscreen (the texture number from the texture manager should be specified)
	void Draw_QuadFS(int textureNum, float alpha);	// Draws a quad with fullscreen with alpha (the texture number from the texture manager should be specified)
	void Draw_QuadFBOFS(int fbo_num);	// Draws a quad with fullscreen (the fbo number from the fbo manager should be specified)
	void Draw_Obj_QuadTex(int textureNum, glm::mat4 *model);

	void Draw_Obj_QuadFBO_Debug(int quad, int fbo_num);
	int tex_tv, tex_isaac2, tex_shotgan, tex_shotgan2, tex_merlucin, tex_xphere;

	// Shaders
	int shdr_QuadTex;		// Draws a textured quad
	int shdr_QuadTexAlpha;	// Draws a textured quad with alpha
	int shdr_QuadTexModel;	// Draws a textured quad with model matrix
private:

	unsigned int obj_quadFullscreen;
	unsigned int obj_quad_FBO_Debug[NUM_FBO_DEBUG];
	string demoDir;
	Resource();

	void Load_Obj_QuadFullscreen();
	void Load_Obj_Quad_FBO_Debug();// Load debugging Quads for FBO
	void Load_Shaders();
	void Load_Tex_Spontz();
	void Load_Text_Fonts();

	static Resource* m_pThis;
};
#endif