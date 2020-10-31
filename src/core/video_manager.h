// videonmanager.h
// Spontz Demogroup

#pragma once

#include "core/video.h"

class VideoManager final {

private:
	using CVideoMap = std::map<CVideoSource, Video*>;

public:
	VideoManager(bool bForceReload);
	~VideoManager();

public:
	// Returns a video (and loads it if required)
	Video* getVideo(CVideoSource const& videoDesc);

private:
	CVideoMap m_videoMap_;
	// Force video reloading on each getVideo call (required for slave mode)
	const bool m_bForceReload_;
};
