// video_manager.cpp
// Spontz Demogroup

#include "video_manager.h"
#include "main.h"

VideoManager::VideoManager(bool bForceReload)
	:
	m_bForceReload(bForceReload)
{
}

VideoManager::~VideoManager()
{
	LOG->Info(LogLevel::MED, "Unloading videos...");
	for (auto const& i : VideoMap_)
		delete i.second;
}

Video* VideoManager::addVideo(CVideoSource const& VideoSource)
{
	// If the video is already loaded just return it
	// If the video is not loaded load and return it
	auto it = VideoMap_.find(VideoSource);

	if (it != VideoMap_.end()) {
		auto pVideo = it->second;
		if (m_bForceReload) {
			// Reload video acording to m_bForceReload
			if (!pVideo->load(VideoSource)) {
				// Handle reload error
				LOG->Error("Could not reload video: \"%s\"", VideoSource.m_sPath.c_str());
				return nullptr;
			}

			LOG->Info(
				LogLevel::MED,
				"Video \"%s\" [id: %d] force reload OK.",
				VideoSource.m_sPath.c_str(),
				VideoMap_.size() - 1
			);

			return pVideo;
		}
	}

	// Create video
	auto const pVideo = new Video(false);

	// Load video
	if (!pVideo->load(VideoSource)) {
		// Handle load error
		LOG->Error("Could not load video: \"%s\"", VideoSource.m_sPath.c_str());
		delete pVideo;
		return nullptr;
	}

	VideoMap_.insert({ VideoSource, pVideo });
	LOG->Info(
		LogLevel::MED,
		"Video \"%s\" [id: %d] loaded OK.",
		VideoSource.m_sPath.c_str(),
		VideoMap_.size() - 1
	);

	return pVideo;
}
