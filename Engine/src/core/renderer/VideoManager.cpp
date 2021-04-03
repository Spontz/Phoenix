// VideoManager.cpp
// Spontz Demogroup

#include "core/renderer/VideoManager.h"
#include "main.h"

namespace Phoenix {

	VideoManager::~VideoManager()
	{
		Logger::info(LogLevel::med, "Unloading videos...");
		for (auto const& i : VideoMap_)
			delete i.second;
	}

	Video* VideoManager::addVideo(CVideoSource const& VideoSource, bool bForceReload)
	{
		// If the video is already loaded just return it
		// If the video is not loaded load and return it
		auto it = VideoMap_.find(VideoSource);

		if (it != VideoMap_.end()) {
			auto pVideo = it->second;
			if (bForceReload) {
				// Reload video acording to m_bForceReload
				if (!pVideo->load(VideoSource)) {
					// Handle reload error
					Logger::error("Could not reload video: \"%s\"", VideoSource.m_sPath.c_str());
					return nullptr;
				}

				Logger::info(
					LogLevel::med,
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
			Logger::error("Could not load video: \"%s\"", VideoSource.m_sPath.c_str());
			delete pVideo;
			return nullptr;
		}

		VideoMap_.insert({ VideoSource, pVideo });
		Logger::info(
			LogLevel::med,
			"Video \"%s\" [id: %d] loaded OK.",
			VideoSource.m_sPath.c_str(),
			VideoMap_.size() - 1
		);

		return pVideo;
	}
}