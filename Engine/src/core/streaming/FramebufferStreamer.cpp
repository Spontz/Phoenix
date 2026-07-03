#include "Main.h"
#include "core/streaming/FramebufferStreamer.h"

#include "rtc/h264rtppacketizer.hpp"
#include "rtc/rtcpnackresponder.hpp"
#include "rtc/rtcpsrreporter.hpp"
#include "rtc/rtppacketizationconfig.hpp"
#include "rtc/rtppacketizer.hpp"
#include "rtc/rtc.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <future>
#include <optional>

namespace Phoenix {
	namespace {
		constexpr uint8_t kPayloadType = 102;
		constexpr uint8_t kAudioPayloadType = 0;
		constexpr uint32_t kVideoSsrc = 1;
		constexpr uint32_t kAudioSsrc = 2;
		constexpr uint32_t kPcmuSampleRate = 8000;
		constexpr uint32_t kPcmuPacketSamples = 160;
		constexpr auto kGatherTimeout = std::chrono::seconds(2);
		constexpr std::array<const char*, 3> kEncoderPresets = { "ultrafast", "veryfast", "faster" };

		int32_t normalizeStreamDimension(uint32_t dimension)
		{
			const auto evenDimension = static_cast<int32_t>(dimension & ~1u);
			return std::max(2, evenDimension);
		}

		int32_t clampEven(int32_t value, int32_t minValue, int32_t maxValue)
		{
			const int32_t clamped = std::clamp(value, minValue, maxValue) & ~1;
			return std::max(minValue, clamped);
		}

		std::pair<int32_t, int32_t> fitStreamDimensions(int32_t sourceWidth, int32_t sourceHeight, const FramebufferStreamer::Settings& settings)
		{
			const int32_t maxWidth = clampEven(settings.maxWidth, 2, 3840);
			const int32_t maxHeight = clampEven(settings.maxHeight, 2, 2160);
			const double scale = std::min({
				1.0,
				static_cast<double>(maxWidth) / static_cast<double>(sourceWidth),
				static_cast<double>(maxHeight) / static_cast<double>(sourceHeight)
			});
			return {
				normalizeStreamDimension(static_cast<uint32_t>(std::max(2.0, std::floor(static_cast<double>(sourceWidth) * scale)))),
				normalizeStreamDimension(static_cast<uint32_t>(std::max(2.0, std::floor(static_cast<double>(sourceHeight) * scale))))
			};
		}

		const char* peerStateName(rtc::PeerConnection::State state)
		{
			switch (state) {
			case rtc::PeerConnection::State::New: return "new";
			case rtc::PeerConnection::State::Connecting: return "connecting";
			case rtc::PeerConnection::State::Connected: return "connected";
			case rtc::PeerConnection::State::Disconnected: return "disconnected";
			case rtc::PeerConnection::State::Failed: return "failed";
			case rtc::PeerConnection::State::Closed: return "closed";
			default: return "unknown";
			}
		}

		const char* iceStateName(rtc::PeerConnection::IceState state)
		{
			switch (state) {
			case rtc::PeerConnection::IceState::New: return "new";
			case rtc::PeerConnection::IceState::Checking: return "checking";
			case rtc::PeerConnection::IceState::Connected: return "connected";
			case rtc::PeerConnection::IceState::Completed: return "completed";
			case rtc::PeerConnection::IceState::Failed: return "failed";
			case rtc::PeerConnection::IceState::Disconnected: return "disconnected";
			case rtc::PeerConnection::IceState::Closed: return "closed";
			default: return "unknown";
			}
		}

		uint8_t linearToMuLaw(float sample)
		{
			constexpr int32_t bias = 0x84;
			constexpr int32_t clip = 32635;
			const int32_t pcm = std::clamp(static_cast<int32_t>(sample * 32767.0f), -clip, clip);
			const int32_t sign = pcm < 0 ? 0x80 : 0x00;
			int32_t magnitude = std::abs(pcm) + bias;
			int32_t exponent = 7;
			for (int32_t mask = 0x4000; (magnitude & mask) == 0 && exponent > 0; mask >>= 1)
				--exponent;
			const int32_t mantissa = (magnitude >> (exponent + 3)) & 0x0f;
			return static_cast<uint8_t>(~(sign | (exponent << 4) | mantissa));
		}
	}

	class WebRtcPreviewSession final {
	public:
		WebRtcPreviewSession()
			:
			m_peer(nullptr),
			m_videoTrack(nullptr),
			m_audioTrack(nullptr),
			m_videoSrReporter(nullptr),
			m_audioSrReporter(nullptr),
			m_videoTrackOpen(false),
			m_audioTrackOpen(false),
			m_videoTrackClosed(false),
			m_answerApplied(false)
		{
		}

		~WebRtcPreviewSession()
		{
			close();
		}

		std::string createOffer(FramebufferStreamer::SignalCallback signalCallback)
		{
			close();
			m_videoTrackOpen = false;
			m_audioTrackOpen = false;
			m_videoTrackClosed = false;
			m_answerApplied = false;

			rtc::Configuration config;
			config.disableAutoNegotiation = true;

			m_peer = std::make_shared<rtc::PeerConnection>(config);
			m_signalCallback = std::move(signalCallback);

			m_peer->onStateChange([this](rtc::PeerConnection::State state) {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview peer state: {}", peerStateName(state));
				if (state == rtc::PeerConnection::State::Disconnected ||
					state == rtc::PeerConnection::State::Failed ||
					state == rtc::PeerConnection::State::Closed) {
					m_videoTrackOpen = false;
					m_audioTrackOpen = false;
					if (m_answerApplied)
						m_videoTrackClosed = true;
				}
			});
			m_peer->onIceStateChange([](rtc::PeerConnection::IceState state) {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview ICE state: {}", iceStateName(state));
			});
			m_peer->onLocalCandidate([this](rtc::Candidate candidate) {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview local ICE candidate: {}", candidate.candidate());
				if (m_signalCallback)
					m_signalCallback("webrtc.ice-candidate", candidate.candidate(), candidate.mid());
			});

			auto videoDescription = rtc::Description::Video("video-stream", rtc::Description::Direction::SendOnly);
			videoDescription.addH264Codec(kPayloadType);
			videoDescription.addSSRC(kVideoSsrc, "video-stream", "phoenix-preview", "video-stream");
			m_videoTrack = m_peer->addTrack(videoDescription);

			auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
				kVideoSsrc,
				"video-stream",
				kPayloadType,
				rtc::H264RtpPacketizer::ClockRate
			);
			auto packetizer = std::make_shared<rtc::H264RtpPacketizer>(
				rtc::NalUnit::Separator::StartSequence,
				rtpConfig
			);
			m_videoSrReporter = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
			packetizer->addToChain(m_videoSrReporter);
			packetizer->addToChain(std::make_shared<rtc::RtcpNackResponder>());
			m_videoTrack->setMediaHandler(packetizer);
			m_videoTrack->onOpen([this]() {
				m_videoTrackOpen = true;
				m_videoTrackClosed = false;
				Logger::info(LogLevel::high, "WebRTC framebuffer preview track opened");
			});
			m_videoTrack->onClosed([this]() {
				m_videoTrackOpen = false;
				if (m_answerApplied)
					m_videoTrackClosed = true;
				Logger::info(LogLevel::high, "WebRTC framebuffer preview track closed");
			});

			auto audioDescription = rtc::Description::Audio("audio-stream", rtc::Description::Direction::SendOnly);
			audioDescription.addPCMUCodec(kAudioPayloadType);
			audioDescription.addSSRC(kAudioSsrc, "audio-stream", "phoenix-preview", "audio-stream");
			m_audioTrack = m_peer->addTrack(audioDescription);
			auto audioRtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(
				kAudioSsrc,
				"audio-stream",
				kAudioPayloadType,
				rtc::PCMURtpPacketizer::DefaultClockRate
			);
			auto audioPacketizer = std::make_shared<rtc::PCMURtpPacketizer>(audioRtpConfig);
			m_audioSrReporter = std::make_shared<rtc::RtcpSrReporter>(audioRtpConfig);
			audioPacketizer->addToChain(m_audioSrReporter);
			m_audioTrack->setMediaHandler(audioPacketizer);
			m_audioTrack->onOpen([this]() {
				m_audioTrackOpen = true;
				Logger::info(LogLevel::high, "WebRTC audio preview track opened");
			});
			m_audioTrack->onClosed([this]() {
				m_audioTrackOpen = false;
				Logger::info(LogLevel::high, "WebRTC audio preview track closed");
			});

			auto answerPromise = std::make_shared<std::promise<std::string>>();
			auto answerFuture = answerPromise->get_future();
			auto completed = std::make_shared<std::atomic_bool>(false);

			m_peer->onGatheringStateChange([peer = m_peer, completed, answerPromise](rtc::PeerConnection::GatheringState state) mutable {
				if (state != rtc::PeerConnection::GatheringState::Complete)
					return;
				if (completed->exchange(true))
					return;

				const auto description = peer->localDescription();
				answerPromise->set_value(description ? std::string(description.value()) : std::string());
			});

			try {
				m_peer->setLocalDescription();
				if (answerFuture.wait_for(kGatherTimeout) == std::future_status::ready)
					return answerFuture.get();

				const auto description = m_peer->localDescription();
				return description ? std::string(description.value()) : std::string();
			}
			catch (const std::exception& err) {
				Logger::error("WebRTC framebuffer preview could not create offer: {}", err.what());
				close();
				return {};
			}
		}

		void handleAnswer(std::string_view sdp)
		{
			if (!m_peer)
				return;

			try {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview applying browser answer");
				m_peer->setRemoteDescription(rtc::Description(std::string(sdp), "answer"));
				m_answerApplied = true;
			}
			catch (const std::exception& err) {
				Logger::error("WebRTC framebuffer preview could not handle answer: {}", err.what());
				close();
			}
		}

		void addRemoteCandidate(std::string_view candidate, std::string_view sdpMid)
		{
			if (!m_peer)
				return;

			try {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview adding remote ICE candidate: {}", candidate);
				m_peer->addRemoteCandidate(rtc::Candidate(std::string(candidate), std::string(sdpMid)));
			}
			catch (const std::exception& err) {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview ignored ICE candidate: {}", err.what());
			}
		}

		bool sendSample(const uint8_t* data, size_t size, int64_t pts, int32_t fps)
		{
			if (!m_videoTrack || size == 0)
				return true;
			if (m_videoTrackClosed)
				return false;
			if (!m_answerApplied || !m_videoTrackOpen || !m_videoTrack->isOpen())
				return true;

			try {
				m_videoTrack->sendFrame(reinterpret_cast<const rtc::byte*>(data), size, std::chrono::duration<double, std::micro>(
					(static_cast<double>(pts) / static_cast<double>(std::max(1, fps))) * 1'000'000.0
				));
				return true;
			}
			catch (const std::exception& err) {
				Logger::info(LogLevel::high, "WebRTC framebuffer preview send failed: {}", err.what());
				m_videoTrackOpen = false;
				m_videoTrackClosed = true;
				return false;
			}
		}

		bool sendAudioSample(const uint8_t* data, size_t size, uint32_t timestamp)
		{
			if (!m_audioTrack || size == 0)
				return true;
			if (!m_answerApplied || !m_audioTrackOpen || !m_audioTrack->isOpen())
				return true;

			try {
				m_audioTrack->sendFrame(reinterpret_cast<const rtc::byte*>(data), size, std::chrono::duration<double>(
					static_cast<double>(timestamp) / static_cast<double>(kPcmuSampleRate)
				));
				return true;
			}
			catch (const std::exception& err) {
				Logger::info(LogLevel::high, "WebRTC audio preview send failed: {}", err.what());
				m_audioTrackOpen = false;
				return false;
			}
		}

		bool isReady() const
		{
			return m_videoTrack && m_videoTrack->isOpen();
		}

		void close()
		{
			m_videoTrackOpen = false;
			m_audioTrackOpen = false;
			m_videoTrackClosed = true;
			m_answerApplied = false;
			if (m_audioTrack) {
				m_audioTrack->close();
				m_audioTrack.reset();
			}
			if (m_videoTrack) {
				m_videoTrack->close();
				m_videoTrack.reset();
			}
			if (m_peer) {
				m_peer->close();
				m_peer.reset();
			}
			m_audioSrReporter.reset();
			m_videoSrReporter.reset();
		}

	private:
		std::shared_ptr<rtc::PeerConnection> m_peer;
		std::shared_ptr<rtc::Track> m_videoTrack;
		std::shared_ptr<rtc::Track> m_audioTrack;
		std::shared_ptr<rtc::RtcpSrReporter> m_videoSrReporter;
		std::shared_ptr<rtc::RtcpSrReporter> m_audioSrReporter;
		FramebufferStreamer::SignalCallback m_signalCallback;
		std::atomic_bool m_videoTrackOpen;
		std::atomic_bool m_audioTrackOpen;
		std::atomic_bool m_videoTrackClosed;
		std::atomic_bool m_answerApplied;
	};

	FramebufferStreamer::FramebufferStreamer()
		:
		m_streamUrl("webrtc://phoenix-preview"),
		m_running(false),
		m_ready(false),
		m_stopRequested(false),
		m_streamFailed(false),
		m_overloadLogged(false),
		m_clientPlaying(false),
		m_audioResamplePosition(0.0),
		m_nextAudioTimestamp(0),
		m_nextCaptureTime(0.0),
		m_nextPts(0),
		m_codecContext(nullptr),
		m_yuvFrame(nullptr),
		m_packet(nullptr),
		m_swsContext(nullptr),
		m_yuvBuffer(nullptr),
		m_streamWidth(0),
		m_streamHeight(0),
		m_streamFps(0),
		m_streamBitrate(0),
		m_streamPreset(0),
		m_swsSourceWidth(0),
		m_swsSourceHeight(0),
		m_settings(),
		m_lastSourceWidth(0),
		m_lastSourceHeight(0),
		m_lastEncodeWidth(0),
		m_lastEncodeHeight(0),
		m_nextSessionId(1),
		m_sessions()
	{
	}

	FramebufferStreamer::~FramebufferStreamer()
	{
		shutdown();
	}

	bool FramebufferStreamer::init()
	{
		if (m_running)
			return true;

		m_stopRequested = false;
		m_streamFailed = false;
		m_overloadLogged = false;
		m_clientPlaying = false;
		m_nextCaptureTime = 0.0;
		m_nextPts = 0;
		m_audioResamplePosition = 0.0;
		m_nextAudioTimestamp = 0;
		m_pendingAudioSamples.clear();

		rtc::InitLogger(rtc::LogLevel::Warning);

		m_running = true;
		m_ready = true;
		m_workerThread = std::thread(&FramebufferStreamer::workerLoop, this);
		Logger::info(LogLevel::high, "WebRTC framebuffer preview streaming enabled");
		return true;
	}

	void FramebufferStreamer::submitFrame(const Viewport& viewport, double nowSeconds)
	{
		submitFrameFromFramebuffer(0, GL_BACK, viewport, nowSeconds);
	}

	void FramebufferStreamer::submitFrameFromFramebuffer(uint32_t framebuffer, uint32_t readBuffer, const Viewport& viewport, double nowSeconds)
	{
		if (!m_running || m_stopRequested || m_streamFailed || !m_clientPlaying)
			return;

		if (viewport.width < 2 || viewport.height < 2)
			return;

		if (nowSeconds < m_nextCaptureTime)
			return;

		Settings settings;
		{
			std::lock_guard lock(m_settingsMutex);
			settings = m_settings;
		}
		settings.fps = std::clamp(settings.fps, 10, 60);
		settings.bitrate = std::clamp(settings.bitrate, 500'000, 50'000'000);
		settings.preset = std::clamp(settings.preset, 0, static_cast<int32_t>(kEncoderPresets.size()) - 1);

		m_nextCaptureTime = nowSeconds + (1.0 / static_cast<double>(settings.fps));

		Frame frame;
		frame.sourceWidth = normalizeStreamDimension(viewport.width);
		frame.sourceHeight = normalizeStreamDimension(viewport.height);
		const auto [encodeWidth, encodeHeight] = fitStreamDimensions(frame.sourceWidth, frame.sourceHeight, settings);
		frame.encodeWidth = encodeWidth;
		frame.encodeHeight = encodeHeight;
		frame.pts = m_nextPts++;
		frame.fps = settings.fps;
		frame.pixels.resize(static_cast<size_t>(frame.sourceWidth) * static_cast<size_t>(frame.sourceHeight) * 3);
		{
			std::lock_guard lock(m_dimensionsMutex);
			m_lastSourceWidth = frame.sourceWidth;
			m_lastSourceHeight = frame.sourceHeight;
			m_lastEncodeWidth = frame.encodeWidth;
			m_lastEncodeHeight = frame.encodeHeight;
		}

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glReadBuffer(static_cast<GLenum>(readBuffer));
		glReadPixels(viewport.x, viewport.y, frame.sourceWidth, frame.sourceHeight, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels.data());

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			while (m_frames.size() >= kMaxQueuedFrames) {
				m_frames.pop_front();
				if (!m_overloadLogged.exchange(true))
					Logger::info(LogLevel::high, "WebRTC framebuffer preview is dropping frames to keep the render loop responsive");
			}
			m_frames.emplace_back(std::move(frame));
		}
		m_queueSignal.notify_one();
	}

	void FramebufferStreamer::submitAudioSamples(const float* samples, uint32_t frameCount, uint32_t channels, uint32_t sampleRate)
	{
		if (!m_running || m_stopRequested || m_streamFailed || !m_clientPlaying)
			return;
		if (!samples || frameCount == 0 || channels == 0 || sampleRate == 0)
			return;

		std::vector<AudioFrame> readyFrames;
		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			const double step = static_cast<double>(sampleRate) / static_cast<double>(kPcmuSampleRate);
			double position = m_audioResamplePosition;
			while (position < static_cast<double>(frameCount)) {
				const uint32_t frameIndex = std::min(static_cast<uint32_t>(position), frameCount - 1);
				const float left = samples[static_cast<size_t>(frameIndex) * channels];
				const float right = channels > 1 ? samples[static_cast<size_t>(frameIndex) * channels + 1] : left;
				m_pendingAudioSamples.push_back(linearToMuLaw((left + right) * 0.5f));
				position += step;
			}
			m_audioResamplePosition = position - static_cast<double>(frameCount);

			while (m_pendingAudioSamples.size() >= kPcmuPacketSamples) {
				AudioFrame audioFrame;
				audioFrame.samples.assign(m_pendingAudioSamples.begin(), m_pendingAudioSamples.begin() + kPcmuPacketSamples);
				audioFrame.timestamp = m_nextAudioTimestamp;
				m_nextAudioTimestamp += kPcmuPacketSamples;
				m_pendingAudioSamples.erase(m_pendingAudioSamples.begin(), m_pendingAudioSamples.begin() + kPcmuPacketSamples);
				readyFrames.emplace_back(std::move(audioFrame));
			}

			for (auto& audioFrame : readyFrames) {
				while (m_audioFrames.size() >= kMaxQueuedAudioFrames)
					m_audioFrames.pop_front();
				m_audioFrames.emplace_back(std::move(audioFrame));
			}
		}

		if (!readyFrames.empty())
			m_queueSignal.notify_one();
	}

	void FramebufferStreamer::shutdown()
	{
		if (!m_running)
			return;

		m_stopRequested = true;
		closePeer();
		m_queueSignal.notify_all();

		if (m_workerThread.joinable())
			m_workerThread.join();

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_frames.clear();
			m_audioFrames.clear();
			m_pendingAudioSamples.clear();
		}

		closeStream();
		m_running = false;
		m_ready = false;
		Logger::info(LogLevel::high, "WebRTC framebuffer preview streaming stopped");
	}

	FramebufferStreamer::Offer FramebufferStreamer::createOffer(SignalCallback signalCallback)
	{
		if (!m_running)
			return {};

		std::lock_guard lock(m_peerMutex);
		const int32_t sessionId = m_nextSessionId++;
		auto session = std::make_shared<WebRtcPreviewSession>();
		const std::string offer = session->createOffer(std::move(signalCallback));
		if (offer.empty())
			return {};

		m_sessions[sessionId] = std::move(session);
		m_clientPlaying = true;
		m_queueSignal.notify_one();
		return { sessionId, offer };
	}

	void FramebufferStreamer::handleAnswer(int32_t sessionId, std::string_view sdp)
	{
		std::lock_guard lock(m_peerMutex);
		const auto it = m_sessions.find(sessionId);
		if (it != m_sessions.end())
			it->second->handleAnswer(sdp);
		else
			Logger::info(LogLevel::high, "WebRTC framebuffer preview ignored answer for missing session {}", sessionId);
	}

	void FramebufferStreamer::handleRemoteCandidate(int32_t sessionId, std::string_view candidate, std::string_view sdpMid, int32_t)
	{
		std::lock_guard lock(m_peerMutex);
		const auto it = m_sessions.find(sessionId);
		if (it != m_sessions.end())
			it->second->addRemoteCandidate(candidate, sdpMid);
		else
			Logger::info(LogLevel::high, "WebRTC framebuffer preview ignored ICE candidate for missing session {}", sessionId);
	}

	FramebufferStreamer::Settings FramebufferStreamer::getSettings() const
	{
		std::lock_guard lock(m_settingsMutex);
		return m_settings;
	}

	void FramebufferStreamer::setSettings(const Settings& settings)
	{
		Settings normalized = settings;
		normalized.maxWidth = clampEven(normalized.maxWidth, 320, 3840);
		normalized.maxHeight = clampEven(normalized.maxHeight, 240, 2160);
		normalized.fps = std::clamp(normalized.fps, 10, 60);
		normalized.bitrate = std::clamp(normalized.bitrate, 500'000, 50'000'000);
		normalized.preset = std::clamp(normalized.preset, 0, static_cast<int32_t>(kEncoderPresets.size()) - 1);

		{
			std::lock_guard lock(m_settingsMutex);
			m_settings = normalized;
		}
		std::lock_guard lock(m_queueMutex);
		m_frames.clear();
	}

	void FramebufferStreamer::mapPreviewPointToSource(float& x, float& y) const
	{
		std::lock_guard lock(m_dimensionsMutex);
		if (m_lastSourceWidth <= 0 || m_lastSourceHeight <= 0 || m_lastEncodeWidth <= 0 || m_lastEncodeHeight <= 0)
			return;

		x *= static_cast<float>(m_lastSourceWidth) / static_cast<float>(m_lastEncodeWidth);
		y *= static_cast<float>(m_lastSourceHeight) / static_cast<float>(m_lastEncodeHeight);
	}

	void FramebufferStreamer::workerLoop()
	{
		while (!m_stopRequested) {
			Frame frame;
			AudioFrame audioFrame;
			bool hasAudioFrame = false;
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_queueSignal.wait(lock, [&] { return m_stopRequested || (m_clientPlaying && (!m_frames.empty() || !m_audioFrames.empty())); });
				if (m_stopRequested)
					break;
				if (!m_clientPlaying || (m_frames.empty() && m_audioFrames.empty()))
					continue;
				if (!m_audioFrames.empty()) {
					audioFrame = std::move(m_audioFrames.front());
					m_audioFrames.pop_front();
					hasAudioFrame = true;
				}
				else {
					frame = std::move(m_frames.front());
					m_frames.pop_front();
				}
			}

			if (hasAudioFrame) {
				sendAudioPacket(audioFrame);
				continue;
			}

			if (!encodeFrame(frame)) {
				std::lock_guard lock(m_queueMutex);
				m_frames.clear();
			}
		}

		closePeer();
		m_ready = false;
	}

	bool FramebufferStreamer::openEncoder(int32_t width, int32_t height, int32_t fps, int32_t bitrate, int32_t preset)
	{
		if (m_codecContext && m_streamWidth == width && m_streamHeight == height && m_streamFps == fps && m_streamBitrate == bitrate && m_streamPreset == preset)
			return true;

		if (m_codecContext)
			Logger::info(LogLevel::high, "WebRTC framebuffer preview resized to {}x{}", width, height);

		closeStream();

		const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
		if (!codec)
			codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!codec) {
			Logger::error("WebRTC framebuffer preview could not start: H.264 encoder not found in this FFMPEG build");
			return false;
		}

		m_codecContext = avcodec_alloc_context3(codec);
		if (!m_codecContext) {
			Logger::error("WebRTC framebuffer preview could not allocate encoder context");
			return false;
		}

		m_codecContext->codec_id = codec->id;
		m_codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
		m_codecContext->width = width;
		m_codecContext->height = height;
		m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		m_codecContext->time_base = AVRational{ 1, fps };
		m_codecContext->framerate = AVRational{ fps, 1 };
		m_codecContext->bit_rate = bitrate;
		m_codecContext->gop_size = fps;
		m_codecContext->max_b_frames = 0;
		m_codecContext->flags |= AV_CODEC_FLAG_LOW_DELAY;
		m_codecContext->thread_count = 1;

		AVDictionary* codecOptions = nullptr;
		av_dict_set(&codecOptions, "preset", kEncoderPresets[static_cast<size_t>(preset)], 0);
		av_dict_set(&codecOptions, "tune", "zerolatency", 0);
		av_dict_set(&codecOptions, "x264-params", "repeat-headers=1:annexb=1:scenecut=0:sync-lookahead=0:rc-lookahead=0:bframes=0:force-cfr=1", 0);

		const int rc = avcodec_open2(m_codecContext, codec, &codecOptions);
		av_dict_free(&codecOptions);
		if (rc < 0) {
			logFfmpegError("WebRTC framebuffer preview could not open encoder", rc);
			return false;
		}

		m_yuvFrame = av_frame_alloc();
		m_packet = av_packet_alloc();
		if (!m_yuvFrame || !m_packet) {
			Logger::error("WebRTC framebuffer preview could not allocate FFMPEG frame buffers");
			return false;
		}

		m_yuvFrame->format = m_codecContext->pix_fmt;
		m_yuvFrame->width = width;
		m_yuvFrame->height = height;

		const int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
		m_yuvBuffer = static_cast<uint8_t*>(av_malloc(yuvSize));
		if (!m_yuvBuffer) {
			Logger::error("WebRTC framebuffer preview could not allocate image buffers");
			return false;
		}
		av_image_fill_arrays(m_yuvFrame->data, m_yuvFrame->linesize, m_yuvBuffer, AV_PIX_FMT_YUV420P, width, height, 1);
		m_streamWidth = width;
		m_streamHeight = height;
		m_streamFps = fps;
		m_streamBitrate = bitrate;
		m_streamPreset = preset;
		return true;
	}

	void FramebufferStreamer::closePeer()
	{
		std::lock_guard lock(m_peerMutex);
		for (auto& [_, session] : m_sessions)
			session->close();
		m_sessions.clear();
		m_clientPlaying = false;
	}

	bool FramebufferStreamer::encodeFrame(const Frame& frame)
	{
		if (frame.pixels.empty())
			return true;

		Settings settings;
		{
			std::lock_guard lock(m_settingsMutex);
			settings = m_settings;
		}
		const int32_t fps = std::clamp(frame.fps, 10, 60);
		const int32_t bitrate = std::clamp(settings.bitrate, 500'000, 50'000'000);
		const int32_t preset = std::clamp(settings.preset, 0, static_cast<int32_t>(kEncoderPresets.size()) - 1);

		if (!openEncoder(frame.encodeWidth, frame.encodeHeight, fps, bitrate, preset))
			return false;

		if (!m_swsContext || m_swsSourceWidth != frame.sourceWidth || m_swsSourceHeight != frame.sourceHeight) {
			if (m_swsContext)
				sws_freeContext(m_swsContext);
			m_swsContext = sws_getContext(frame.sourceWidth, frame.sourceHeight, AV_PIX_FMT_RGB24, m_streamWidth, m_streamHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, nullptr, nullptr, nullptr);
			m_swsSourceWidth = frame.sourceWidth;
			m_swsSourceHeight = frame.sourceHeight;
			if (!m_swsContext) {
				Logger::error("WebRTC framebuffer preview could not create pixel conversion context");
				return false;
			}
		}

		const uint8_t* sourceData[1] = { frame.pixels.data() + static_cast<size_t>(frame.sourceHeight - 1) * static_cast<size_t>(frame.sourceWidth) * 3 };
		const int sourceLinesize[1] = { -frame.sourceWidth * 3 };
		sws_scale(m_swsContext, sourceData, sourceLinesize, 0, frame.sourceHeight, m_yuvFrame->data, m_yuvFrame->linesize);

		m_yuvFrame->pts = frame.pts;

		int rc = avcodec_send_frame(m_codecContext, m_yuvFrame);
		if (rc < 0) {
			logFfmpegError("WebRTC framebuffer preview could not send frame to encoder", rc);
			return false;
		}

		while (rc >= 0) {
			rc = avcodec_receive_packet(m_codecContext, m_packet);
			if (rc == AVERROR(EAGAIN) || rc == AVERROR_EOF)
				return true;
			if (rc < 0) {
				logFfmpegError("WebRTC framebuffer preview could not receive encoded packet", rc);
				return false;
			}
			const bool sent = sendEncodedPacket(m_packet, fps);
			av_packet_unref(m_packet);
			if (!sent)
				return false;
		}
		return true;
	}

	bool FramebufferStreamer::sendEncodedPacket(const AVPacket* packet, int32_t fps)
	{
		std::vector<std::pair<int32_t, std::shared_ptr<WebRtcPreviewSession>>> sessions;
		{
			std::lock_guard lock(m_peerMutex);
			sessions.reserve(m_sessions.size());
			for (const auto& [sessionId, session] : m_sessions)
				sessions.emplace_back(sessionId, session);
		}

		if (sessions.empty())
			return true;

		std::vector<int32_t> failedSessions;
		for (const auto& [sessionId, session] : sessions) {
			if (!session->sendSample(packet->data, static_cast<size_t>(packet->size), packet->pts < 0 ? 0 : packet->pts, fps))
				failedSessions.push_back(sessionId);
		}

		if (!failedSessions.empty()) {
			std::lock_guard lock(m_peerMutex);
			for (const int32_t sessionId : failedSessions)
				m_sessions.erase(sessionId);
			m_clientPlaying = !m_sessions.empty();
		}

		return true;
	}

	bool FramebufferStreamer::sendAudioPacket(const AudioFrame& frame)
	{
		std::vector<std::pair<int32_t, std::shared_ptr<WebRtcPreviewSession>>> sessions;
		{
			std::lock_guard lock(m_peerMutex);
			sessions.reserve(m_sessions.size());
			for (const auto& [sessionId, session] : m_sessions)
				sessions.emplace_back(sessionId, session);
		}

		if (sessions.empty())
			return true;

		std::vector<int32_t> failedSessions;
		for (const auto& [sessionId, session] : sessions) {
			if (!session->sendAudioSample(frame.samples.data(), frame.samples.size(), frame.timestamp))
				failedSessions.push_back(sessionId);
		}

		if (!failedSessions.empty()) {
			std::lock_guard lock(m_peerMutex);
			for (const int32_t sessionId : failedSessions)
				m_sessions.erase(sessionId);
			m_clientPlaying = !m_sessions.empty();
		}

		return true;
	}

	void FramebufferStreamer::closeStream()
	{
		if (m_swsContext) {
			sws_freeContext(m_swsContext);
			m_swsContext = nullptr;
		}
		if (m_packet) {
			av_packet_free(&m_packet);
			m_packet = nullptr;
		}
		if (m_yuvFrame) {
			av_frame_free(&m_yuvFrame);
			m_yuvFrame = nullptr;
		}
		if (m_yuvBuffer) {
			av_free(m_yuvBuffer);
			m_yuvBuffer = nullptr;
		}
		if (m_codecContext) {
			avcodec_free_context(&m_codecContext);
			m_codecContext = nullptr;
		}
		m_swsSourceWidth = 0;
		m_swsSourceHeight = 0;
		m_streamWidth = 0;
		m_streamHeight = 0;
		m_streamFps = 0;
		m_streamBitrate = 0;
		m_streamPreset = 0;
	}

	void FramebufferStreamer::logFfmpegError(std::string_view message, int errorCode) const
	{
		char errorBuffer[AV_ERROR_MAX_STRING_SIZE] = {};
		av_strerror(errorCode, errorBuffer, sizeof(errorBuffer));
		Logger::error("{}: {}", message, errorBuffer);
	}

	int FramebufferStreamer::interruptCallback(void* opaque)
	{
		const auto streamer = static_cast<FramebufferStreamer*>(opaque);
		return streamer && streamer->m_stopRequested ? 1 : 0;
	}
}
