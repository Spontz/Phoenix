// FramebufferStreamer.h
// Streams the final OpenGL framebuffer over RTSP.

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Phoenix {

	struct Viewport;

	class FramebufferStreamer final {
	public:
		FramebufferStreamer();
		~FramebufferStreamer();

		bool init();
		void submitFrame(const Viewport& viewport, double nowSeconds);
		void shutdown();

		bool isRunning() const { return m_running; }
		bool isReady() const { return m_ready; }
		const std::string& getStreamUrl() const { return m_streamUrl; }

	private:
		struct Frame {
			std::vector<uint8_t> pixels;
			int32_t width = 0;
			int32_t height = 0;
			int64_t pts = 0;
		};

		void workerLoop();
		bool openEncoder(int32_t width, int32_t height);
		bool openListener();
		bool waitForClient();
		bool handleClientRequest(const std::string& request, bool& playing);
		bool sendRtspResponse(int cseq, int statusCode, std::string_view statusText, std::string_view headers = "", std::string_view body = "");
		bool encodeFrame(const Frame& frame);
		bool sendEncodedPacket(const struct AVPacket* packet);
		bool sendNalUnit(const uint8_t* nal, size_t size, uint32_t timestamp, bool marker);
		bool sendRtpPayload(const uint8_t* payload, size_t size, uint32_t timestamp, bool marker);
		void closeStream();
		void closeClient();
		void closeListener();
		void logFfmpegError(std::string_view message, int errorCode) const;
		static int interruptCallback(void* opaque);

	private:
		static constexpr int32_t kRtspPort = 554;
		static constexpr const char* kRtspPath = "/";
		static constexpr int32_t kStreamFps = 30;
		static constexpr int32_t kStreamBitrate = 4'000'000;
		static constexpr size_t kMaxQueuedFrames = 1;
		static constexpr size_t kMaxRtpPayload = 1200;

		std::string m_streamUrl;
		std::atomic_bool m_running;
		std::atomic_bool m_ready;
		std::atomic_bool m_stopRequested;
		std::atomic_bool m_streamFailed;
		std::atomic_bool m_overloadLogged;
		std::atomic_bool m_clientPlaying;
		std::atomic_bool m_winsockStarted;

		std::thread m_workerThread;
		std::mutex m_queueMutex;
		std::condition_variable m_queueSignal;
		std::deque<Frame> m_frames;
		double m_nextCaptureTime;
		int64_t m_nextPts;

		struct AVCodecContext* m_codecContext;
		struct AVFrame* m_yuvFrame;
		struct AVPacket* m_packet;
		struct SwsContext* m_swsContext;
		uint8_t* m_yuvBuffer;
		int32_t m_streamWidth;
		int32_t m_streamHeight;
		int32_t m_swsSourceWidth;
		int32_t m_swsSourceHeight;
		std::atomic<uintptr_t> m_listenSocket;
		std::atomic<uintptr_t> m_clientSocket;
		uint16_t m_rtpSequence;
		uint32_t m_rtpSsrc;
		std::string m_sessionId;
	};
}
