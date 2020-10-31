// videomanager.cpp
// Spontz Demogroup

#include "video_manager.h"
#include "main.h"

VideoManager::VideoManager(bool bForceReload)
	:
	m_bForceReload_(bForceReload)
{
}

VideoManager::~VideoManager()
{
	for (auto pVideo : m_videos_)
		delete pVideo;
}

// Adds a Texture into the queue, returns the Number of the texture added
Video* VideoManager::addVideo(std::string const& sPath, int32_t iVideoStreamIndex)
{
	// If video already exists return or reload and return it (acording to m_forceLoad)
	for (auto const pVideo : m_videos_) {
		if (pVideo->getFileName() == sPath) {
			if (m_bForceReload_) {
				// Reload video
				if (!pVideo->load(sPath, iVideoStreamIndex)) {
					// Handle reload error
					LOG->Error("Could not reload video: \"%s\"", sPath.c_str());
					return nullptr;
				}

				LOG->Info(
					LogLevel::MED,
					"Video \"%s\" [id: %d] force reload OK.",
					sPath.c_str(),
					m_videos_.size() - 1
				);

				return pVideo;
			}
		}
	}

	// Create video
	auto const pVideo = new Video(false, 10, 1.0); // hack: hardcoded pars

	// Load video
	if (!pVideo->load(sPath, iVideoStreamIndex)) {
		// Handle load error
		LOG->Error("Could not load video: \"%s\"", sPath.c_str());
		delete pVideo;
		return nullptr;
	}

	m_videos_.push_back(pVideo);
	LOG->Info(LogLevel::MED, "Video \"%s\" [id: %d] loaded OK.", sPath.c_str(), m_videos_.size() - 1);

	return pVideo;
}

