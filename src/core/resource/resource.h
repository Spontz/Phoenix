// resouce.h
// Spontz Demogroup

#ifndef RESOURCE_H
#define RESOURCE_H
#include <string>
/*
#include <glm/glm.hpp>
#include <glad/glad.h>
*/
//#include <glm/gtc/matrix_transform.hpp>

#include "main.h"

#define NUM_FBO_DEBUG 4

using namespace std;
#define RES Resource::GetResource()

class Resource {
public:
	static Resource* GetResource();
	void loadAllResources();

	void Draw_QuadFS();											// Draws a quad in fullscreen
	void Draw_QuadFS(int textureNum);							// Draws a quad with fullscreen (the texture number from the texture manager should be specified)
	void Draw_QuadFS(int textureNum, float alpha);				// Draws a quad with fullscreen with alpha (the texture number from the texture manager should be specified)
	void Draw_QuadFBOFS(int fbo_num, GLuint attachment = 0);	// Draws a quad with fullscreen (the fbo number from the fbo manager should be specified)
	void Draw_Obj_QuadTex(int textureNum, glm::mat4 *model);
	void Draw_Obj_QuadTex(int textureNum, glm::mat4 *view, glm::mat4 *projection, glm::mat4 *model);
	void Draw_Skybox(int cubemap);								// Draws a skybox for a given cubemap texture
	void Draw_Cube();											// Draws a cube

	void Draw_Obj_QuadFBO_Debug(int quad, int fbo_num, int fbo_attachment);
	int tex_tv; // Default texture (tv)

	// Shaders
	int shdr_QuadTex;			// Draws a textured quad
	int shdr_QuadDepth;			// Draws a depth texture quad
	int shdr_QuadTexAlpha;		// Draws a textured quad with alpha
	int shdr_QuadTexModel;		// Draws a textured quad with model matrix
	int shdr_QuadTexVFlipModel;	// Draws a textured quad with model matrix, and with the texture flipped vertically
	int shdr_Skybox;			// Draws a skybox
	int shdr_ObjColor;			// Draws an object with a color

	// Objects
	GLuint obj_skybox;
	GLuint obj_quadFullscreen; // TODO: Temporal, move to private
	GLuint obj_qube;
	
private:

	unsigned int obj_quad_FBO_Debug[NUM_FBO_DEBUG];
	Resource();

	void Load_Obj_QuadFullscreen();
	void Load_Obj_Quad_FBO_Debug();// Load debugging Quads for FBO
	void Load_Obj_Skybox();
	void Load_Obj_Qube();
	void Load_Shaders();
	void Load_Tex_Spontz();
	void Load_Text_Fonts();

	void Load_Lights();


	static Resource* m_pThis;
};
#endif