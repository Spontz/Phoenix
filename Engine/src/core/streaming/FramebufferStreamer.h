// FramebufferStreamer.h
// Streams the final OpenGL framebuffer to the browser preview.

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace Phoenix {

	struct Viewport;

	class FramebufferStreamer final {
	public:
		FramebufferStreamer();
		~FramebufferStreamer();

		using SignalCallback = std::function<void(std::string_view type, std::string_view payload, std::string_view sdpMid)>;
		struct Offer {
			int32_t sessionId = 0;
			std::string sdp;
		};
		struct Settings {
			int32_t maxWidth = 1280;
			int32_t maxHeight = 720;
			int32_t fps = 30;
			int32_t bitrate = 8'000'000;
			int32_t preset = 0;
		};

		bool init();
		void submitFrame(const Viewport& viewport, double nowSeconds);
		void shutdown();
		Offer createOffer(SignalCallback signalCallback);
		void handleAnswer(int32_t sessionId, std::string_view sdp);
		void handleRemoteCandidate(int32_t sessionId, std::string_view candidate, std::string_view sdpMid, int32_t sdpMLineIndex);
		Settings getSettings() const;
		void setSettings(const Settings& settings);
		void mapPreviewPointToSource(float& x, float& y) const;

		bool isRunning() const { return m_running; }
		bool isReady() const { return m_ready; }
		const std::string& getStreamUrl() const { return m_streamUrl; }

	private:
		struct Frame {
			std::vector<uint8_t> pixels;
			int32_t sourceWidth = 0;
			int32_t sourceHeight = 0;
			int32_t encodeWidth = 0;
			int32_t encodeHeight = 0;
			int64_t pts = 0;
			int32_t fps = 30;
		};

		void workerLoop();
		bool openEncoder(int32_t width, int32_t height, int32_t fps, int32_t bitrate, int32_t preset);
		void closePeer();
		bool encodeFrame(const Frame& frame);
		bool sendEncodedPacket(const struct AVPacket* packet, int32_t fps);
		void closeStream();
		void logFfmpegError(std::string_view message, int errorCode) const;
		static int interruptCallback(void* opaque);

	private:
		static constexpr size_t kMaxQueuedFrames = 1;

		std::string m_streamUrl;
		std::atomic_bool m_running;
		std::atomic_bool m_ready;
		std::atomic_bool m_stopRequested;
		std::atomic_bool m_streamFailed;
		std::atomic_bool m_overloadLogged;
		std::atomic_bool m_clientPlaying;

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
		int32_t m_streamFps;
		int32_t m_streamBitrate;
		int32_t m_streamPreset;
		int32_t m_swsSourceWidth;
		int32_t m_swsSourceHeight;
		mutable std::mutex m_settingsMutex;
		Settings m_settings;
		mutable std::mutex m_dimensionsMutex;
		int32_t m_lastSourceWidth;
		int32_t m_lastSourceHeight;
		int32_t m_lastEncodeWidth;
		int32_t m_lastEncodeHeight;
		std::mutex m_peerMutex;
		int32_t m_nextSessionId;
		std::unordered_map<int32_t, std::shared_ptr<class WebRtcPreviewSession>> m_sessions;
	};
}
