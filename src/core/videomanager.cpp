// videomanager.cpp
// Spontz Demogroup

#include "videomanager.h"
#include "main.h"

VideoManager::~VideoManager()
{
	m_forceReload = false;
	for (auto const& pVideo : m_videos_)
		delete pVideo;
}

// Adds a Texture into the queue, returns the Number of the texture added
Video* VideoManager::addVideo(std::string const& sPath, int32_t iVideoStreamIndex)
{
	// If video already exists return or reload and return it (acording to m_forceLoad)
	for (auto const pVideo : m_videos_) {
		if (pVideo->getFileName() == sPath) {
			if (m_forceReload) {
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

Video* VideoManager::getVideo(int32_t iVideoIndex) const
{
	if (iVideoIndex >= 0 && iVideoIndex < m_videos_.size())
		return m_videos_[iVideoIndex];

	return nullptr;
}
