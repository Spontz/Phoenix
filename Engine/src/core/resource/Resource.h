// Resource.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Texture.h"
#include "core/renderer/Cubemap.h"
#include "core/renderer/Fbo.h"
#include "core/renderer/Shader.h"
#include "core/renderer/VertexArray.h"

namespace Phoenix {

	class Resource {
	public:
		Resource();
		virtual ~Resource();

		demokernel& m_demo;

		void loadAllResources();
		void unLoadAllResources();

		void Draw_QuadFS();											// Draws a quad in fullscreen
		void Draw_QuadFS(Texture* tex, float alpha);				// Draws a quad in fullscreen with alpha
		void Draw_QuadFBOFS(Fbo* fbo, GLuint attachment = 0);		// Draws a quad in fullscreen (the fbo number from the fbo manager should be specified)
		void Draw_QuadEfxFBOFS(int efxFboNum, GLuint attachment = 0);	// Draws a quad in fullscreen (the fbo number from the efx fbo manager should be specified)
		void Draw_Obj_QuadTex(Texture* tex, glm::mat4 const* model);
		void Draw_Obj_QuadTex(Texture* tex, glm::mat4 const* projection, glm::mat4 const* view, glm::mat4 const* model);
		void Draw_Skybox(Cubemap* cubemap);							// Draws a skybox for a given cubemap texture
		void Draw_Cube();											// Draws a cube
		void Draw_Grid(glm::vec3 const color, glm::mat4 const* MVP);

		Texture* m_pTVImage;				// Default texture (tv)

		// Shaders
		Shader* m_pShdrQuadTex;				// Draws a textured quad
		Shader* m_pShdrQuadDepth;			// Draws a depth texture quad
		Shader* m_pShdrQuadTexAlpha;		// Draws a textured quad with alpha
		Shader* m_pShdrQuadTexModel;		// Draws a textured quad with model matrix
		Shader* m_pShdrQuadTexPVM;			// Draws a textured quad with projection, view and model matrixes
		Shader* m_pShdrQuadTexVFlipModel;	// Draws a textured quad with model matrix, and with the texture flipped vertically
		Shader* m_pShdrSkybox;				// Draws a skybox
		Shader* m_pShdrObjColor;			// Draws an object with a color
		Shader* m_pShdrGrid;				// Draws a Grid

		// Vertex Array Objects
		VertexArray* m_pSkybox;
		VertexArray* m_pQuadFullScreen;
		VertexArray* m_pQube;
		VertexArray* m_pGrid;

		// Grid properties
		void		Load_Grid();
		float		m_gridSize;
		int			m_gridSlices;

	private:
		void Load_Obj_QuadFullscreen();
		void Load_Obj_Skybox();
		void Load_Obj_Qube();
		void Load_Shaders();
		void Load_Tex_Spontz();
		void Load_Text_Fonts();
		void Load_Lights();

		void safeDelete(void* ptr);
	};
}