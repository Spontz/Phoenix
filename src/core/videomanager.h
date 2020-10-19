// videonmanager.h
// Spontz Demogroup

#pragma once

#include "core/video.h"

class VideoManager {
public:
	~VideoManager();

public:
	Video* addVideo(std::string const& path, int videoStreamIndex);
	Video* getVideo(int videoIndex) const;

public:
	bool				forceLoad;	// Force video loading each time we add a video (should be enabled when working on slave mode)

private:
	std::vector<Video*> m_videos_;	// Video list
};
