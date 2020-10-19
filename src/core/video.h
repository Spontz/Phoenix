// video.h
// Spontz Demogroup

#pragma once

#include "libs.h"

#include <thread>

class Video {
public:
	Video(
		bool bDebug = false,
		uint32_t iDecodingThreadCount = 10, // ideally logical cores - 1
		double dPlaybackSpeed = 1.0 // 1.0 means normal speed, 2.0 double speed, etc. 
	);

	~Video();

public:
	bool load(
		std::string const& sFileName,
		int32_t iVideoStreamIndex			// -1 will use the first video stream found the file
	);

	void renderVideo(double dTime);			// Render the video to the OpenGL texture at the specified time
	void bind(GLuint uiIndex = 0) const;
	std::string const& getFileName() const;
	GLuint getTexID() const;
	int32_t getWidth() const;
	int32_t getHeight() const;

private:
	void clearData();
	double renderInterval() const;
	int64_t seekTime(double dTime);
	int32_t decodePacket();

private:
	std::string	m_sFileName_;					// Video file name
	int32_t m_iVideoStreamIndex_;				// Video stream index used for playback
	double m_dFramerate_;						// Video framerate
	int32_t m_iWidth_;							// Video width
	int32_t m_iHeight_;							// Video height
	GLuint m_uiTexID_;							// OpenGL texture ID where the video is displayed
	bool m_bLoaded_;
	AVFormatContext* m_pFormatContext_;			// AVFormatContext holds the header information from the format (Container)
	AVCodec* m_pAVCodec_;						// The component that knows how to encode and decode the stream is the codec
	AVCodecParameters* m_pAVCodecParameters_;	// This component describes the properties of a codec used by the stream i
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
	bool m_bStopWorkerThread_;

	const bool m_bDebug_;
	const uint32_t m_uiDecodingThreadCount_;
	const double m_dPlaybackSpeed_;
};
