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
		Video*	addVideo(CVideoSource const& videoDesc, bool ForceReload); // TODO: Change the video to shared pointer, as we do for Textures, models, etc...
		void	clear();

	private:
		CVideoMap VideoMap_;
	};
}