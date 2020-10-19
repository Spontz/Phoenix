// videomanager.cpp
// Spontz Demogroup

#include "videomanager.h"
#include "main.h"

VideoManager::~VideoManager()
{
	forceLoad = false;
	for (auto const& pVideo : m_videos_)
		delete pVideo;
}

// Adds a Texture into the queue, returns the Number of the texture added
Video* VideoManager::addVideo(std::string const& path, int videoStreamIndex)
{
	Video* p_video = nullptr;
	// Return ID if the video has already been loaded
	for (auto i = 0; i < m_videos_.size(); ++i) {
		if (m_videos_[i]->getFileName().compare(path) == 0) {
			p_video = m_videos_[i];
		}
	}
	
	if (p_video == nullptr) {
		p_video = new Video(false, 10, 1.0); // hack: pars should come from section parameters
		if (p_video->load(path, videoStreamIndex)) {
			m_videos_.push_back(p_video);
			LOG->Info(LogLevel::MED, "Video \"%s\" [id: %d] loaded OK.", path.c_str(), m_videos_.size() - 1);
		}
		else {
			LOG->Error("Could not load video: \"%s\"", path.c_str());
			delete p_video;
			p_video = nullptr;
		}
	}
	// If the texture is catched we should not do anything, unless we have been told to upload it again
	else {
		if (forceLoad) {
			if (p_video->load(path, videoStreamIndex)) {
				LOG->Info(LogLevel::MED, "Video \"%s\" [id: %d] force reload OK.", path.c_str(), m_videos_.size() - 1);
			}
			else {
				LOG->Error("Could not load video: \"%s\"", path.c_str());
			}
		}
	}

	return p_video;
}

Video* VideoManager::getVideo(int videoIndex) const
{
	if (videoIndex >= 0 && videoIndex < m_videos_.size())
		return m_videos_[videoIndex];
	else
		return nullptr;
}
