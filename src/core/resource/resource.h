// Resource.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Texture.h"
#include "core/renderer/Cubemap.h"
#include "core/renderer/Shader.h"
#include "core/renderer/VertexArray.h"

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

	Texture* m_pTVImage;			// Default texture (tv)

	// Shaders
	Shader* m_pShdrQuadTex;				// Draws a textured quad
	Shader* m_pShdrQuadDepth;			// Draws a depth texture quad
	Shader* m_pShdrQuadTexAlpha;		// Draws a textured quad with alpha
	Shader* m_pShdrQuadTexModel;		// Draws a textured quad with model matrix
	Shader* m_pShdrQuadTexPVM;			// Draws a textured quad with projection, view and model matrixes
	Shader* m_pShdrQuadTexVFlipModel;	// Draws a textured quad with model matrix, and with the texture flipped vertically
	Shader* m_pShdrSkybox;				// Draws a skybox
	Shader* m_pShdrObjColor;			// Draws an object with a color

	// Vertex Array Objects
	VertexArray* m_pSkybox;
	VertexArray* m_pQuadFullScreen;
	VertexArray* m_pQube;
	
private:
	void Load_Obj_QuadFullscreen();
	void Load_Obj_Skybox();
	void Load_Obj_Qube();
	void Load_Shaders();
	void Load_Tex_Spontz();
	void Load_Text_Fonts();
	void Load_Lights();
};
