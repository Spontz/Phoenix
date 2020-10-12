// videomanager.cpp
// Spontz Demogroup

#include "videomanager.h"
#include "main.h"

VideoManager::~VideoManager()
{
	for (auto const& pVideo : videos_)
		delete pVideo;
}

// Adds a Texture into the queue, returns the Number of the texture added
int VideoManager::addVideo(std::string const& path)
{
	// Return ID if the video has already been loaded
	for (auto i = 0; i < videos_.size(); ++i)
		if (videos_[i]->getFileName().compare(path) == 0)
			return i;

	// Otherwise load the video
	const auto pNewVideo = new Video();
	if (pNewVideo->load(path)) {
		videos_.push_back(pNewVideo);
		const int videoNum = static_cast<int>(videos_.size()) - 1;
		LOG->Info(LogLevel::MED, "Video \"%s\" [id: %d] loaded OK.", path.c_str(), videoNum);
		return videoNum;
	}
	else {
		LOG->Error("Could not load video: \"%s\"", path.c_str());
		delete pNewVideo;
		return -1;
	}
}

Video* VideoManager::getVideo(int videoIndex) const
{
	return videos_[videoIndex]; 
}
