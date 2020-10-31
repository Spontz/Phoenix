// videonmanager.h
// Spontz Demogroup

#pragma once

#include "core/video.h"

class VideoManager final {
public:
	VideoManager(bool bForceReload);
	~VideoManager();

public:
	Video* addVideo(std::string const& sPath, int32_t iVideoStreamIndex);

private:
	std::vector<Video*> m_videos_;
	/// Force video loading each time we add a video (should be enabled on slave mode)
	const bool m_bForceReload_;
};
