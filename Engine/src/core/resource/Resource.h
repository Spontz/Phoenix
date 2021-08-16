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

		void Draw_QuadFS();												// Draws a quad in fullscreen
		void Draw_QuadFS(SP_Texture tex, float alpha);					// Draws a quad in fullscreen with alpha
		void Draw_QuadFBOFS(Fbo* fbo, GLuint attachment = 0);			// Draws a quad in fullscreen (the fbo number from the fbo manager should be specified)
		void Draw_QuadEfxFBOFS(int efxFboNum, GLuint attachment = 0);	// Draws a quad in fullscreen (the fbo number from the efx fbo manager should be specified)
		void Draw_Obj_QuadTex(SP_Texture tex, glm::mat4 const* model);
		void Draw_Obj_QuadTex(SP_Texture tex, glm::mat4 const* projection, glm::mat4 const* view, glm::mat4 const* model);
		void Draw_Skybox(SP_Cubemap cubemap);							// Draws a skybox for a given cubemap texture
		void Draw_Cube();												// Draws a cube
		void Draw_Grid(glm::vec3 const color, glm::mat4 const* MVP);

		SP_Texture m_pTVImage;				// Default texture (tv)

		// Shaders
		SP_Shader m_pShdrQuadTex;			// Draws a textured quad
		SP_Shader m_pShdrQuadDepth;			// Draws a depth texture quad
		SP_Shader m_pShdrQuadTexAlpha;		// Draws a textured quad with alpha
		SP_Shader m_pShdrQuadTexModel;		// Draws a textured quad with model matrix
		SP_Shader m_pShdrQuadTexPVM;		// Draws a textured quad with projection, view and model matrixes
		SP_Shader m_pShdrQuadTexVFlipModel;	// Draws a textured quad with model matrix, and with the texture flipped vertically
		SP_Shader m_pShdrSkybox;			// Draws a skybox
		SP_Shader m_pShdrObjColor;			// Draws an object with a color
		SP_Shader m_pShdrGrid;				// Draws a Grid

		// Vertex Array Objects
		SP_VertexArray m_pSkybox;
		SP_VertexArray m_pQuadFullScreen;
		SP_VertexArray m_pQube;
		SP_VertexArray m_pGrid;

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