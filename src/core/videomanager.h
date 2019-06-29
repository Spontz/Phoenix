// videonmanager.h
// Spontz Demogroup

#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H
#include <string>
#include <vector>
#include "core/video.h"

using namespace std;

class VideoManager {
public:
	std::vector<Video*> video;	// Video list

	VideoManager();
	int addVideo(string path);
	int	getOpenGLTextureID(int index);

private:
	
};

#endif