// video_manager.cpp
// Spontz Demogroup

#include "video_manager.h"
#include "main.h"

VideoManager::VideoManager(bool bForceReload)
	:
	bForceReload_(bForceReload)
{
}

VideoManager::~VideoManager()
{
	LOG->Info(LogLevel::MED, "Unloading videos...");
	for (auto const& i : VideoMap_)
		delete i.second;
}

Video* VideoManager::getVideo(CVideoSource const& VideoSource)
{
	// If the video is already loaded just return it
	// If the video is not loaded load and return it
	auto it = VideoMap_.find(VideoSource);

	if (it != VideoMap_.end()) {
		auto pVideo = it->second;
		if (bForceReload_) {
			// Reload video acording to m_bForceReload_
			if (!pVideo->load(VideoSource)) {
				// Handle reload error
				LOG->Error("Could not reload video: \"%s\"", VideoSource.sPath_.c_str());
				return nullptr;
			}

			LOG->Info(
				LogLevel::MED,
				"Video \"%s\" [id: %d] force reload OK.",
				VideoSource.sPath_.c_str(),
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
		LOG->Error("Could not load video: \"%s\"", VideoSource.sPath_.c_str());
		delete pVideo;
		return nullptr;
	}

	VideoMap_.insert({ VideoSource, pVideo });
	LOG->Info(
		LogLevel::MED,
		"Video \"%s\" [id: %d] loaded OK.",
		VideoSource.sPath_.c_str(),
		VideoMap_.size() - 1
	);

	return pVideo;
}

