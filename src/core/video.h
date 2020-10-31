// video.h
// Spontz Demogroup

#pragma once

#include "libs.h"

#include <thread>

/// <summary>
/// Video asset source
/// </summary>
class CVideoSource final {
public:
	bool operator<(CVideoSource const& value) const {
		if (sPath_ < value.sPath_)
			return true;
		if (iVideoStreamIndex_ < value.iVideoStreamIndex_)
			return true;
		if (uiDecodingThreadCount_ < value.uiDecodingThreadCount_)
			return true;
		return (dPlaybackSpeed_ < value.dPlaybackSpeed_);
	}

public:
	std::string sPath_;
	int32_t iVideoStreamIndex_ = -1;		// -1 will use the first video stream found the file
	uint32_t uiDecodingThreadCount_ = 10;	// ideally logical cores - 1
	double dPlaybackSpeed_ = 1.0;			// 1.0 means normal speed, 2.0 double speed, etc.
};

/// <summary>
/// Encapsulates video rendering to a ogl texture via FFMPEG.
/// </summary>
class Video final {
public:
	Video(bool bDebug = false);
	~Video();

public:
	[[nodiscard]] bool load(CVideoSource const& videoDesc);
	void renderVideo(double dTime);							// Render the video to the OpenGL texture at the specified time

	void bind(GLuint uiTexUnit) const;
	[[nodiscard]] std::string const& getFileName() const;
	[[nodiscard]] GLuint getTexID() const;
	[[nodiscard]] int32_t getWidth() const;
	[[nodiscard]] int32_t getHeight() const;

private:
	void clearData();
	void decode();
	int32_t decodePacket();
	[[nodiscard]] double renderInterval() const;
	int64_t seekTime(double dSeconds) const;				// Returns the frame number

private:
	CVideoSource VideoSource_;
	double dFramerate_;										// Video framerate
	int32_t iWidth_;										// Video width
	int32_t iHeight_;										// Video height
	GLuint uiTextureOGLName_;								// OpenGL texture ID where the video is displayed
	bool bLoaded_;
	AVFormatContext* pFormatContext_;						// AVFormatContext holds the format header information (Container)
	AVCodec* pAVCodec_;										// The component that knows how to encode and decode the stream
	AVCodecParameters* pAVCodecParameters_;					// This component describes the properties of a codec used by the stream
	AVCodecContext* pCodecContext_;							// Codec context
	AVFrame* pFrame_;										// AV Frame
	AVFrame* pGLFrame_;										// OpenGL Frame
	SwsContext* pConvertContext_;							// Convert Context (for OpenGL)
	AVPacket* pAVPacket_;									// Packet
	double dIntervalFrame_;									// Time between frames (1/frameRate)
	double dNextFrameTime_;									// Time to present the next frame
	std::thread* pWorkerThread_;							// Video decoding thread
	bool bNewFrame_;
	double dTime_;
	bool bStopWorkerThread_;
	const bool bDebug_;
};
