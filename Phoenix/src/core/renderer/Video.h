// Video.h
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
	int32_t m_iVideoStreamIndex = -1;  // -1 will use the first video stream found the file
	uint32_t m_uiDecodingThreadCount = 10;  // ideally logical cores - 1
	double m_dPlaybackSpeed = 1.0;  // 1.0 means normal speed, 2.0 double speed, etc.
};

/// <summary>
/// Encapsulates video rendering to a ogl texture via FFMPEG.
/// </summary>
class Video final {
public:
	Video(bool bDebug = false);
	~Video();

public:
	void bind(GLuint uiTexUnit) const;
	std::string const& getFileName() const;
	GLuint getTexID() const;
	int32_t getWidth() const;
	int32_t getHeight() const;
	bool load(CVideoSource const& videoDesc);
	void renderVideo(double dTime);  // Render the video to the OpenGL texture at the specified time

private:
	void clearData();
	void decode();
	int32_t decodePacket();
	double renderInterval() const;
	int64_t seekTime(double dSeconds) const;  // Seek video and returns the frame number

private:
	const bool m_bDebug;

private:
	CVideoSource m_VideoSource;
	double m_dFramerate; // In frames/sec
	int32_t m_iWidth;
	int32_t m_iHeight;
	GLuint m_uiTextureOGLName;  // OpenGL texture name storing video frames
	bool m_bLoaded;
	AVFormatContext* m_pFormatContext;  // Holds the format header information (Container)
	AVCodec* m_pAVCodec;
	AVCodecParameters* m_pAVCodecParameters;
	AVCodecContext* m_pCodecContext;
	AVFrame* m_pFrame;
	AVFrame* m_pGLFrame;
	SwsContext* m_pConvertContext;
	AVPacket* m_pAVPacket;
	double m_dIntervalFrame;  // Time in seconds between frames (1/frameRate)
	double m_dNextFrameTime;  // Time to in seconds present the next frame
	std::thread* m_pWorkerThread;
	bool m_bNewFrame;
	double m_dTime;
	bool m_bStopWorkerThread;
};
