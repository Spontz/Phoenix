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
	void renderVideo(double dTime);			// Render the video to the OpenGL texture at the specified time
	void bind(int index = 0) const;
	std::string const& getFileName() const;
	GLuint getTexID() const;
	int getWidth() const;
	int getHeight() const;

private:
	void seekTime(double dTime);
	int decodePacket();

private:
	std::string	m_fileName_;				// Video file name
	double m_dFramerate_;						// Video framerate
	int m_width_;							// Video width
	int m_height_;							// Video height
	GLuint m_texID_;						// OpenGL texture ID where the video is displayed
	bool m_loaded_;
	AVFormatContext* m_pFormatContext_;		// AVFormatContext holds the header information from the format (Container)
	AVCodec* m_pCodec_;						// The component that knows how to encode and decode the stream is the codec
	AVCodecParameters* m_pCodecParameters_;	// This component describes the properties of a codec used by the stream i
	int m_videoStreamIndex_;				// Stream to read
	AVCodecContext* m_pCodecContext_;		// Codec context
	AVFrame* m_pFrame_;						// AV Frame
	AVFrame* m_pGLFrame_;					// OpenGL Frame
	SwsContext* m_pConvertContext_;			// Convert Context (for OpenGL)
	AVPacket* m_pAVPacket_;					// Packet
	double m_dIntervalFrame_;				// Time between frames (1/frameRate)
	double m_dNextFrameTime_;				// Last time we rendered a frame
};
