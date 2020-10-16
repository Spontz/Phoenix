// video.h
// Spontz Demogroup

#pragma once

#include "libs.h"

#include <mutex>

class Video {
public:
	Video(bool debug=false, uint32_t decodingThreadCount=10, double playbackSpeed=1.0);
	~Video();

public:
	bool load(std::string const& filename, int videoStreamIndex /* -1 means using the first video stream we found */);
	void renderVideo(double dTime);			// Render the video to the OpenGL texture at the specified time
	void bind(int index = 0) const;
	std::string const& getFileName() const;
	GLuint getTexID() const;
	int getWidth() const;
	int getHeight() const;

private:
	double renderInterval() const;
	void seekTime(double dTime);
	int decodePacket();

private:
	std::string	m_fileName_;					// Video file name
	double m_dFramerate_;						// Video framerate
	int m_width_;								// Video width
	int m_height_;								// Video height
	GLuint m_texID_;							// OpenGL texture ID where the video is displayed
	bool m_loaded_;
	AVFormatContext* m_pFormatContext_;			// AVFormatContext holds the header information from the format (Container)
	AVCodec* m_pAVCodec_;						// The component that knows how to encode and decode the stream is the codec
	AVCodecParameters* m_pAVCodecParameters_;	// This component describes the properties of a codec used by the stream i
	int m_videoStreamIndex_;					// Video stream index used for playback
	AVCodecContext* m_pCodecContext_;			// Codec context
	AVFrame* m_pFrame_;							// AV Frame
	AVFrame* m_pGLFrame_;						// OpenGL Frame
	SwsContext* m_pConvertContext_;				// Convert Context (for OpenGL)
	AVPacket* m_pAVPacket_;						// Packet
	double m_dIntervalFrame_;					// Time between frames (1/frameRate)
	double m_dNextFrameTime_;					// Last time we rendered a frame
	std::thread* m_pWorkerThread_;				// Video decoding thread
	bool m_bNewFrame_;
	double m_dTime_;
	bool m_stopWorkerThread_;
	const bool m_debug_;
	const uint32_t m_decodingThreadCount_;
	const double m_playbackSpeed_;
	// std::mutex m_mutex_;
};
