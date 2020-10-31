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
	LOG->Info(LogLevel::MED, "Unloading videos...");
	for (auto const& i : m_videoMap_)
		delete i.second;
}

Video* VideoManager::getVideo(CVideoSource const& videoDesc)
{
	// If the video is already loaded just return it
	// If the video is not loaded load and return it
	auto it = m_videoMap_.find(videoDesc);

	if (it != m_videoMap_.end()) {
		auto pVideo = it->second;
		if (m_bForceReload_) {
			// Reload video acording to m_bForceReload_
			if (!pVideo->load(videoDesc)) {
				// Handle reload error
				LOG->Error("Could not reload video: \"%s\"", videoDesc.sPath_.c_str());
				return nullptr;
			}

			LOG->Info(
				LogLevel::MED,
				"Video \"%s\" [id: %d] force reload OK.",
				videoDesc.sPath_.c_str(),
				m_videoMap_.size() - 1
			);

			return pVideo;
		}
	}

	// Create video
	auto const pVideo = new Video(false);

	// Load video
	if (!pVideo->load(videoDesc)) {
		// Handle load error
		LOG->Error("Could not load video: \"%s\"", videoDesc.sPath_.c_str());
		delete pVideo;
		return nullptr;
	}

	m_videoMap_.insert({ videoDesc, pVideo });
	LOG->Info(
		LogLevel::MED,
		"Video \"%s\" [id: %d] loaded OK.",
		videoDesc.sPath_.c_str(),
		m_videoMap_.size() - 1
	);

	return pVideo;
}

