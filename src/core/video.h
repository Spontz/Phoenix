// video.h
// Spontz Demogroup

#ifndef VIDEO_H
#define VIDEO_H
#include <string>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
using namespace std;

class Video {
public:
	string	fileName;	// Video filename
	float	framerate;	// Video framerate
	int		width;		// Video width
	int		height;		// Video height
	GLuint	texID;		// OpenGL texture ID where is displayed the video

	Video();
	virtual ~Video();

	bool load(const std::string & filename);
	void renderVideo(float time);	// Render the video to the OpenGL texture at the specified time
	void bind(int index = 0) const;
			
private:
	bool loaded;
	AVFormatContext *pFormatContext;		// AVFormatContext holds the header information from the format (Container)
	AVCodec *pCodec;						// The component that knows how to enCOde and DECode the stream is the codec
	AVCodecParameters *pCodecParameters;	// This component describes the properties of a codec used by the stream i
	int video_stream_index;					// Stream to read
	AVCodecContext *pCodecContext;			// Codec context
	AVFrame *pFrame;						// AV Frame
	AVFrame *glFrame;						// OpenGL Frame
	SwsContext *conv_ctx;					// Convert Context (for OpenGL)
	AVPacket *pPacket;						// Packet
	float intervalFrame;					// Time between frames (1/frameRate)
	float lastRenderTime;					// Last time we rendered a frame
	
	void seekTime(float time);
	int decodePacket();

};

#endif