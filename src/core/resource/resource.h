// resouce.h
// Spontz Demogroup

#pragma once

#include <core/texture.h>
#include <core/cubemap.h>
#include <core/shader.h>

class Resource {
public:
	Resource();
	virtual ~Resource();

	demokernel& m_demo;

	void loadAllResources();

	void Draw_QuadFS();											// Draws a quad in fullscreen
	void Draw_QuadFS(int textureNum);							// Draws a quad in fullscreen (the texture number from the texture manager should be specified) // TODO: remove this and use the one with the texture object (see below)
	void Draw_QuadFS(int textureNum, float alpha);				// Draws a quad in fullscreen with alpha (the texture number from the texture manager should be specified) // TODO: remove this and use the one with the texture object (see below)
	void Draw_QuadFS(Texture* tex, float alpha);				// Draws a quad in fullscreen with alpha
	void Draw_QuadFBOFS(int fbo_num, GLuint attachment=0);		// Draws a quad in fullscreen (the fbo number from the fbo manager should be specified)
	void Draw_QuadEfxFBOFS(int efxFboNum, GLuint attachment=0);	// Draws a quad in fullscreen (the fbo number from the efx fbo manager should be specified)
	void Draw_Obj_QuadTex(int textureNum, glm::mat4 const* model);
	void Draw_Obj_QuadTex(Texture* tex, glm::mat4 const* model);	// TODO: Use this and remove the others not using the Texture object
	void Draw_Obj_QuadTex(int textureNum, glm::mat4 *projection, glm::mat4* view, glm::mat4 *model);
	void Draw_Skybox(Cubemap* cubemap);							// Draws a skybox for a given cubemap texture
	void Draw_Cube();											// Draws a cube

	Texture* tex_tv; // Default texture (tv)

	// Shaders
	Shader* shdr_QuadTex;			// Draws a textured quad
	Shader* shdr_QuadDepth;			// Draws a depth texture quad
	Shader* shdr_QuadTexAlpha;		// Draws a textured quad with alpha
	Shader* shdr_QuadTexModel;		// Draws a textured quad with model matrix
	Shader* shdr_QuadTexPVM;		// Draws a textured quad with projection, view and model matrixes
	Shader* shdr_QuadTexVFlipModel;	// Draws a textured quad with model matrix, and with the texture flipped vertically
	Shader* shdr_Skybox;			// Draws a skybox
	Shader* shdr_ObjColor;			// Draws an object with a color

	// Objects
	GLuint obj_skybox;
	GLuint obj_quadFullscreen; // TODO: Temporal, move to private
	GLuint obj_qube;
	
private:
	void Load_Obj_QuadFullscreen();
	void Load_Obj_Skybox();
	void Load_Obj_Qube();
	void Load_Shaders();
	void Load_Tex_Spontz();
	void Load_Text_Fonts();

	void Load_Lights();
};
