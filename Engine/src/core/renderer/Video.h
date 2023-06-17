// Video.h
// Spontz Demogroup

#pragma once

#include "libs.h"

#include <thread>

namespace Phoenix {

	// Video asset source
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

	// Encapsulates video rendering to a ogl texture via FFMPEG.
	class Video;
	using SP_Video = std::shared_ptr<Video>;
	using WP_Video = std::weak_ptr<Video>;

	class Video final {
	public:
		Video(bool bDebug = false);
		~Video();

	public:
		void resetData();
		void bind(GLuint uiTexUnit) const;
		std::string const& getFileName() const;
		GLuint getTexID() const;
		int32_t getWidth() const;
		int32_t getHeight() const;
		bool load(CVideoSource const& videoDesc);
		void unload();

		void renderVideo(double dTime);  // Render the video to the OpenGL texture at the specified time
		// warning: there are videos with variable framerate
		double avgFramePeriod() const
		{
			return 1.0 / avgFrameRate();
		}

		// warning: there are videos with variable framerate
		double avgFrameRate() const
		{
			return av_q2d(m_pFormatContext->streams[m_VideoSource.m_iVideoStreamIndex]->avg_frame_rate);
		}

		bool loaded() const
		{
			return m_WorkerThread.joinable();
		}

		double videoDurationSecs() const
		{
			return static_cast<double>(m_pFormatContext->duration) / AV_TIME_BASE;
		}

	private:
		void decode();
		int32_t decodePacket();
		int64_t seekTime(double dSeconds) const;  // Seek video and returns the frame number

	private:
		const bool m_bDebug;

	private:
		CVideoSource m_VideoSource;
		int32_t m_iWidth;
		int32_t m_iHeight;
		int64_t m_numFrames = -1;  // Video fame count
		GLuint m_uiTextureOGLName;  // OpenGL texture name storing video frames
		AVFormatContext* m_pFormatContext;  // Holds the format header information (Container)
		const AVCodec* m_pAVCodec;
		AVCodecParameters* m_pAVCodecParameters;
		AVCodecContext* m_pCodecContext;
		AVFrame* m_pFrame;
		AVFrame* m_pGLFrame;
		const uint8_t* m_puiInternalBuffer;
		SwsContext* m_pConvertContext;
		AVPacket* m_pAVPacket;
		double m_dNextFrameTime;  // Time in seconds present the next frame
		std::thread m_WorkerThread;
		std::atomic_bool m_bNewFrame;
		double m_dTime;
		std::atomic_bool m_bStopWorkerThread;
	};
}
