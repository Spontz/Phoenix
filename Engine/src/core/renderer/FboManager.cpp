// FboManager.cpp
// Spontz Demogroup

#include "main.h"
#include "core/renderer/FboManager.h"

namespace Phoenix {

	// Init vars
	FboManager::FboManager()
		:
		mem(0),
		currentFbo(-1),
		clearColor(false),
		clearDepth(false)
	{
		fbo.clear();
	}

	void FboManager::init()
	{
	}

	void FboManager::bind(const int32_t fbo_num, const bool newClearColor, const bool newClearDepth)
	{
		if (fbo_num < fbo.size()) {
			currentFbo = fbo_num;
			clearColor = newClearColor;
			clearDepth = newClearDepth;
			Fbo* my_fbo = fbo[fbo_num];
			// Adjust the viewport to the fbo size
			DEMO->m_Window->SetCurrentViewport({ 0,0,static_cast<unsigned int>(my_fbo->width),static_cast<unsigned int>(my_fbo->height) });
			my_fbo->bind();
			if (clearColor)	glClear(GL_COLOR_BUFFER_BIT);
			if (clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
		}
	}

	// Bind the current fbo: useful for some efects that need to change the framebuffer (like Boom or blur)
	void FboManager::bindCurrent()
	{
		if (currentFbo >= 0) {
			Fbo* my_fbo = fbo[currentFbo];
			// Adjust the viewport to the fbo size
			DEMO->m_Window->SetCurrentViewport({ 0,0,static_cast<unsigned int>(my_fbo->width),static_cast<unsigned int>(my_fbo->height) });
			my_fbo->bind();
			if (clearColor)	glClear(GL_COLOR_BUFFER_BIT);
			if (clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
		}
		else {
			unbind(clearColor, clearDepth);
		}
	}

	void FboManager::bind_tex(int32_t fbo_num, GLuint texUnit, GLuint attachment)
	{
		if (fbo_num < fbo.size())
			fbo[fbo_num]->bind_tex(texUnit, attachment);
	}

	void FboManager::unbind(const bool newClearColor, const bool newClearDepth)
	{
		currentFbo = -1;
		clearColor = newClearColor;
		clearDepth = newClearDepth;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Restore the driver viewport
		DEMO->m_Window->SetCurrentViewport(DEMO->m_Window->GetFramebufferViewport());
		if (clearColor)	glClear(GL_COLOR_BUFFER_BIT);
		if (clearDepth)	glClear(GL_DEPTH_BUFFER_BIT);
	}

	// Adds a Fbo into the queue, returns the ID of the texture added
	int FboManager::addFbo(const FboConfig& cfg)
	{
		int fbo_id = -1;
		int iformat = getInternalFormat(cfg.format);
		int format = getFormat(cfg.format);
		int type = getType(cfg.format);
		int components = getComponents(cfg.format);
		int colorAttachments = cfg.numColorAttachments;
		bool useBilinearFilter = cfg.useBilinearFilter;

		
		Fbo* new_fbo = new Fbo();
		if (new_fbo->upload(cfg.format, static_cast<int>(cfg.width), static_cast<int>(cfg.height), iformat, format, type, colorAttachments, useBilinearFilter)) {
			fbo.push_back(new_fbo);
			mem += (float)(new_fbo->width * new_fbo->height * components) / 1048576.0f;		// increase the texture mem
			fbo_id = (int)fbo.size() - 1;
			Logger::info(LogLevel::med, "Fbo Color {} loaded OK. Overall fbo Memory: {:.3f}Mb", fbo_id, mem);
		}
		else {
			Logger::error("Could not load fbo with the current format: Width: {}, Height: {}, iformat: {}, format: {}, type: {}, Filter: {}", cfg.width, cfg.height, iformat, format, type, useBilinearFilter);
			return -1;
		}
		return fbo_id;
	}

	void FboManager::clearFbos()
	{
		for (auto& pFbo : fbo)
			delete pFbo;
		
		fbo.clear();
		mem = 0;
	}

	void FboManager::clearFbosColor()
	{
		for (auto& pFbo : fbo)
			pFbo->clearColor();
	}

	int FboManager::getOpenGLTextureID(uint32_t index, uint32_t attachment)
	{
		if (index < fbo.size()) {
			if (attachment < fbo[index]->numAttachments) {
				return fbo[index]->m_colorAttachment[attachment];
			}
			else
				return -1;
		}
		else
			return -1;
	}


	int FboManager::getFormat(std::string const& name) {
		for (int i = 0; i < FboFormat.size(); i++) {
			if (name == FboFormat[i].name) {
				return FboFormat[i].tex_format;
			}
		}
		return -1;
	}

	int FboManager::getInternalFormat(std::string const& name) {
		for (int i = 0; i < FboFormat.size(); i++) {
			if (name == FboFormat[i].name) {
				return FboFormat[i].tex_iformat;
			}
		}
		return -1;
	}

	int FboManager::getType(std::string const& name) {
		for (int i = 0; i < FboFormat.size(); i++) {
			if (name == FboFormat[i].name) {
				return FboFormat[i].tex_type;
			}
		}
		return -1;
	}

	int FboManager::getComponents(std::string const& name) {
		for (int i = 0; i < FboFormat.size(); i++) {
			if (name == FboFormat[i].name) {
				return FboFormat[i].tex_components;
			}
		}
		return -1;
	}
}