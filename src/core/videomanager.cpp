// videomanager.cpp
// Spontz Demogroup

#include "videomanager.h"
#include "main.h"

VideoManager::~VideoManager()
{
	for (auto const& pVideo : m_videos_)
		delete pVideo;
}

// Adds a Texture into the queue, returns the Number of the texture added
int VideoManager::addVideo(std::string const& path, int videoStreamIndex)
{
	// Return ID if the video has already been loaded
	for (auto i = 0; i < m_videos_.size(); ++i)
		if (m_videos_[i]->getFileName().compare(path) == 0)
			return i;

	// Otherwise load the video
	const auto pNewVideo = new Video(false, 10, 1.0); // hack: pars should come from section parameters
	if (pNewVideo->load(path, videoStreamIndex)) {
		m_videos_.push_back(pNewVideo);
		const int videoNum = static_cast<int>(m_videos_.size()) - 1;
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
	return m_videos_[videoIndex]; 
}
