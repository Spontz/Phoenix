// Resource.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Texture.h"
#include "core/renderer/Cubemap.h"
#include "core/renderer/Fbo.h"
#include "core/renderer/Shader.h"
#include "core/renderer/VertexArray.h"

namespace Phoenix {

	class Resource final {
	public:
		Resource();
		~Resource();

	public:
		void loadAllResources();
		void unloadAllResources();

		void drawQuadFS(); // Draws a quad in fullscreen
		void drawQuadFS(SP_Texture spTex, float alpha); // Draws a quad in fullscreen with alpha
		// Draws a quad in fullscreen (the fbo number from the fbo manager should be specified)
		void drawQuadFboFS(Fbo* pFbo, GLuint attachment = 0);
		// Draws a quad in fullscreen (the fbo number from the efx fbo manager should be specified)
		void drawQuadEfxFboFS(int32_t efxFboNum, GLuint attachment = 0);
		void drawObjQuadTex(SP_Texture spTex, glm::mat4 const* pWorld);
		void drawObjQuadTex(
			SP_Texture spTex,
			glm::mat4 const* pProj,
			glm::mat4 const* pView,
			glm::mat4 const* pWorld
		);
		void drawSkybox(SP_Cubemap spCubemap); // Draws a skybox for a given cubemap texture
		void drawCube(); // Draws a cube
		void drawOneGrid(glm::vec3 const& color, glm::mat4 const* pWVP);
		void draw3DGrid(bool drawAxisX, bool drawAxisY, bool drawAxisZ);
		void loadGrid();

	private:
		void loadObjQuadFullscreen();
		void loadObjSkybox();
		void loadObjCube();
		void loadShaders();
		void loadTexSpontz();
		void loadLights();

	public:
		float m_gridSize;
		int32_t m_gridSlices;

		DemoKernel& m_demo;

		SP_Texture m_spTVImage; // Default texture (tv)

		// Shaders
		SP_Shader m_spShdrQuadTex; // Textured quad
		SP_Shader m_spShdrQuadDepth; // Depth texture quad
		SP_Shader m_spShdrQuadTexAlpha; // Textured quad with alpha
		SP_Shader m_spShdrQuadTexModel; // Textured quad with model matrix
		SP_Shader m_spShdrQuadTexPVM; // Textured quad with projection, view and model matrixes
		SP_Shader m_spShdrQuadTexVFlipModel; // Quad with model matrix, vertically fliped texture
		SP_Shader m_spShdrSkybox; // Skybox
		SP_Shader m_spShdrObjColor; // Object with a color
		SP_Shader m_spShdrGrid; // Grid

		// Vertex Array Objects
		SP_VertexArray m_spSkybox;
		SP_VertexArray m_spQuadFullScreen;
		SP_VertexArray m_spCube;
		SP_VertexArray m_spGrid;
	};
}
