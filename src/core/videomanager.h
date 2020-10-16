// videonmanager.h
// Spontz Demogroup

#pragma once

#include "core/video.h"

class VideoManager {
public:
	~VideoManager();

public:
	int addVideo(std::string const& path, int videoStreamIndex);
	Video* getVideo(int videoIndex) const;

private:
	std::vector<Video*> m_videos_; // Video list
};
