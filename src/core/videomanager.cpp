// videomanager.cpp
// Spontz Demogroup

#include "main.h"
#include "videomanager.h"


// Init vars
VideoManager::VideoManager() {
	video.clear();
}

VideoManager::~VideoManager()
{
	video.clear();
}

// Adds a Texture into the queue, returns the Number of the texture added
int VideoManager::addVideo(std::string path) {
	
	unsigned int i;
	int video_num = -1;

	// check if video is already loaded, then we just return the ID of our video
	for (i = 0; i < video.size(); i++) {
		if (video[i]->fileName.compare(path) == 0) {
			return i;
		}
	}
	// if we must load the video...
	Video *new_vid = new Video();
	if (new_vid->load(path)) {
		video.push_back(new_vid);
		video_num = (int)video.size() - 1;
		LOG->Info(LogLevel::MED, "Video %s [id: %d] loaded OK.", path.c_str(), video_num);
	}
	else
		LOG->Error("Could not load vido: %s", path.c_str());
	
	return video_num;
}

int VideoManager::getOpenGLTextureID(int index)
{
	if (index < video.size())
		return video[index]->texID;
	return -1;
}
