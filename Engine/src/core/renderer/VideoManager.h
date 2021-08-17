// VideoManager.h
// Spontz Demogroup

#pragma once

#include "core/renderer/Video.h"

namespace Phoenix {

	class VideoManager final {

	private:
		using CVideoMap = std::map<CVideoSource, SP_Video>;

	public:
		~VideoManager();

	public:
		// Returns a video (and loads it if required)
		SP_Video	addVideo(CVideoSource const& videoDesc, bool ForceReload);
		void		clear();

	private:
		CVideoMap VideoMap_;
	};
}