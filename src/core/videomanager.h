// videonmanager.h
// Spontz Demogroup

#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H
#include <string>
#include <vector>
#include "core/video.h"

class VideoManager {
public:
	std::vector<Video*> video;	// Video list

	VideoManager();
	virtual ~VideoManager();

	int addVideo(std::string path);
	int	getOpenGLTextureID(int index);

private:
	
};

#endif