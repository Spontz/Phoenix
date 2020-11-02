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
		if (m_sPath < value.m_sPath)
			return true;
		if (m_iVideoStreamIndex < value.m_iVideoStreamIndex)
			return true;
		if (m_uiDecodingThreadCount < value.m_uiDecodingThreadCount)
			return true;
		return (m_dPlaybackSpeed < value.m_dPlaybackSpeed);
	}

public:
	std::string m_sPath;
	int32_t m_iVideoStreamIndex = -1;		// -1 will use the first video stream found the file
	uint32_t m_uiDecodingThreadCount = 10;	// ideally logical cores - 1
	double m_dPlaybackSpeed = 1.0;			// 1.0 means normal speed, 2.0 double speed, etc.
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
	CVideoSource m_VideoSource;
	double m_dFramerate;									// Video framerate
	int32_t m_iWidth;										// Video width
	int32_t m_iHeight;										// Video height
	GLuint m_uiTextureOGLName;								// OpenGL texture ID where the video is displayed
	bool m_bLoaded;
	AVFormatContext* m_pFormatContext;						// AVFormatContext holds the format header information (Container)
	AVCodec* m_pAVCodec;									// The component that knows how to encode and decode the stream
	AVCodecParameters* m_pAVCodecParameters;				// This component describes the properties of a codec used by the stream
	AVCodecContext* m_pCodecContext;						// Codec context
	AVFrame* m_pFrame;										// AV Frame
	AVFrame* m_pGLFrame;									// OpenGL Frame
	SwsContext* m_pConvertContext;							// Convert Context (for OpenGL)
	AVPacket* m_pAVPacket;									// Packet
	double m_dIntervalFrame;								// Time between frames (1/frameRate)
	double m_dNextFrameTime;								// Time to present the next frame
	std::thread* m_pWorkerThread;							// Video decoding thread
	bool m_bNewFrame;
	double m_dTime;
	bool m_bStopWorkerThread;
	const bool m_bDebug;
};
