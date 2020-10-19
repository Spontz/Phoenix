// videonmanager.h
// Spontz Demogroup

#pragma once

#include "core/video.h"

class VideoManager final {
public:
	~VideoManager();

public:
	Video* addVideo(std::string const& sPath, int32_t iVideoStreamIndex);
	Video* getVideo(int32_t iVideoIndex) const;

public:
	// Force video loading each time we add a video (should be enabled when working on slave mode)
	bool				m_forceReload;

private:
	std::vector<Video*> m_videos_;	// Video list
};
