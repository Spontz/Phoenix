// VideoManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Video.h"

namespace Phoenix {

	class VideoManager final {

	private:
		using CVideoMap = std::map<CVideoSource, Video*>;

	public:
		VideoManager() {};
		~VideoManager();

	public:
		// Returns a video (and loads it if required)
		Video* addVideo(CVideoSource const& videoDesc, bool ForceReload);

	private:
		CVideoMap VideoMap_;
	};
}