// video.h
// Spontz Demogroup

#ifndef VIDEO_H
#define VIDEO_H
#include <string>
using namespace std;

class Video {
public:
	string	folder;		// Where video is stored?
	int		totalFps;	// Number of FPS
	float	fps;		// Video fps's
	float	duration;	// Duration of the video (seconds)

	Video();
	virtual ~Video();

	bool load(const std::string & folderPath, bool flip);
	void bind(int index = 0) const;
			
private:
	vector<int>	textureNum;	// Texture number (from the texture manager)

};

#endif