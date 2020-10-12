// video.h
// Spontz Demogroup

#pragma once

#include "libs.h"

class Video {
public:
	Video();
	~Video();

public:
	bool load(std::string const& filename);
	void renderVideo(float time);			// Render the video to the OpenGL texture at the specified time
	void bind(int index = 0) const;

public:
	std::string const& getFileName() const { return fileName; }
	GLuint getTexID() const { return texID; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }

private:
	void seekTime(float time);
	int decodePacket();

private:
	std::string	fileName;					// Video file name
	float		framerate;					// Video framerate
	int			width;						// Video width
	int			height;						// Video height
	GLuint		texID;						// OpenGL texture ID where the video is displayed

private:
	bool loaded;
	AVFormatContext *pFormatContext;		// AVFormatContext holds the header information from the format (Container)
	AVCodec *pCodec;						// The component that knows how to encode and decode the stream is the codec
	AVCodecParameters *pCodecParameters;	// This component describes the properties of a codec used by the stream i
	int VideoStreamIndex;					// Stream to read
	AVCodecContext *pCodecContext;			// Codec context
	AVFrame *pFrame;						// AV Frame
	AVFrame *pGLFrame;						// OpenGL Frame
	SwsContext *pConvertContext;			// Convert Context (for OpenGL)
	AVPacket *pPacket;						// Packet
	float intervalFrame;					// Time between frames (1/frameRate)
	float lastRenderTime;					// Last time we rendered a frame
};
