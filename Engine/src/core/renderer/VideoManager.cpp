// VideoManager.cpp
// Spontz Demogroup

#include "core/renderer/VideoManager.h"
#include "main.h"

namespace Phoenix {

	VideoManager::~VideoManager()
	{
		clear();
	}

	SP_Video VideoManager::addVideo(CVideoSource const& VideoSource, bool bForceReload)
	{
		// If the video is already loaded just return it
		// If the video is not loaded load and return it
		auto it = VideoMap_.find(VideoSource);

		if (it != VideoMap_.end()) {
			auto pVideo = it->second;
			if (bForceReload) {
				if (pVideo->loaded()) {
					pVideo->unload();
				}
				// Reload video acording to m_bForceReload
				if (!pVideo->load(VideoSource)) {
					// Handle reload error
					Logger::error("Could not reload video: \"{}\"", VideoSource.m_sPath);
					return nullptr;
				}

				Logger::info(
					LogLevel::med,
					"Video \"{}\" [id: {}] force reload OK.",
					VideoSource.m_sPath,
					VideoMap_.size() - 1
				);

				return pVideo;
			}
		}

		// Create video
		SP_Video pVideo = std::make_shared<Video>(false);

		// Load video
		if (!pVideo->load(VideoSource)) {
			// Handle load error
			Logger::error("Could not load video: \"{}\"", VideoSource.m_sPath);
			return nullptr;
		}

		VideoMap_.insert({ VideoSource, pVideo });
		Logger::info(
			LogLevel::med,
			"Video \"{}\" [id: {}] loaded OK.",
			VideoSource.m_sPath.c_str(),
			VideoMap_.size() - 1
		);

		return pVideo;
	}
	void VideoManager::clear()
	{
		Logger::info(LogLevel::med, "Unloading videos...");
		VideoMap_.clear();
	}
}