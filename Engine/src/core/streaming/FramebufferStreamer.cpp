#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include "Main.h"
#include "core/streaming/FramebufferStreamer.h"

#include <algorithm>

namespace Phoenix {
	namespace {
		constexpr SOCKET kInvalidSocket = INVALID_SOCKET;

		std::string getHeaderValue(const std::string& request, const std::string& header)
		{
			const auto pattern = header + ":";
			auto pos = request.find(pattern);
			if (pos == std::string::npos)
				return {};
			pos += pattern.size();
			while (pos < request.size() && request[pos] == ' ')
				++pos;
			const auto end = request.find("\r\n", pos);
			return request.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
		}

		int getCSeq(const std::string& request)
		{
			const auto value = getHeaderValue(request, "CSeq");
			return value.empty() ? 1 : std::max(1, atoi(value.c_str()));
		}

		bool sendAll(SOCKET socket, const uint8_t* data, size_t size)
		{
			size_t sent = 0;
			while (sent < size) {
				const auto chunk = static_cast<int>(std::min<size_t>(size - sent, 16 * 1024));
				const auto rc = send(socket, reinterpret_cast<const char*>(data + sent), chunk, 0);
				if (rc == SOCKET_ERROR || rc == 0)
					return false;
				sent += static_cast<size_t>(rc);
			}
			return true;
		}

		bool findStartCode(const uint8_t* data, size_t size, size_t from, size_t& pos, size_t& codeSize)
		{
			for (size_t i = from; i + 3 < size; ++i) {
				if (data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 1) {
					pos = i;
					codeSize = 3;
					return true;
				}
				if (i + 4 < size && data[i] == 0 && data[i + 1] == 0 && data[i + 2] == 0 && data[i + 3] == 1) {
					pos = i;
					codeSize = 4;
					return true;
				}
			}
			return false;
		}

		int32_t normalizeStreamDimension(uint32_t dimension)
		{
			const auto evenDimension = static_cast<int32_t>(dimension & ~1u);
			return std::max(2, evenDimension);
		}
	}

	FramebufferStreamer::FramebufferStreamer()
		:
		m_streamUrl(std::format("rtsp://0.0.0.0:{}{}", kRtspPort, kRtspPath)),
		m_running(false),
		m_ready(false),
		m_stopRequested(false),
		m_streamFailed(false),
		m_overloadLogged(false),
		m_clientPlaying(false),
		m_winsockStarted(false),
		m_nextCaptureTime(0.0),
		m_nextPts(0),
		m_codecContext(nullptr),
		m_yuvFrame(nullptr),
		m_packet(nullptr),
		m_swsContext(nullptr),
		m_yuvBuffer(nullptr),
		m_streamWidth(0),
		m_streamHeight(0),
		m_swsSourceWidth(0),
		m_swsSourceHeight(0),
		m_listenSocket(static_cast<uintptr_t>(kInvalidSocket)),
		m_clientSocket(static_cast<uintptr_t>(kInvalidSocket)),
		m_rtpSequence(0),
		m_rtpSsrc(0x50485831),
		m_sessionId("PHOENIX")
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

		Logger::info(LogLevel::high, "RTSP framebuffer stream starting at rtsp://localhost:{} (single VLC client)", kRtspPort);

		m_running = true;
		m_workerThread = std::thread(&FramebufferStreamer::workerLoop, this);
		return true;
	}

	void FramebufferStreamer::submitFrame(const Viewport& viewport, double nowSeconds)
	{
		if (!m_running || m_stopRequested || m_streamFailed || !m_clientPlaying)
			return;

		if (viewport.width < 2 || viewport.height < 2)
			return;

		if (nowSeconds < m_nextCaptureTime)
			return;

		m_nextCaptureTime = nowSeconds + (1.0 / static_cast<double>(kStreamFps));

		Frame frame;
		frame.width = normalizeStreamDimension(viewport.width);
		frame.height = normalizeStreamDimension(viewport.height);
		frame.pts = m_nextPts++;
		frame.pixels.resize(static_cast<size_t>(frame.width) * static_cast<size_t>(frame.height) * 3);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_BACK);
		glReadPixels(viewport.x, viewport.y, frame.width, frame.height, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels.data());

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			while (m_frames.size() >= kMaxQueuedFrames) {
				m_frames.pop_front();
				if (!m_overloadLogged.exchange(true))
					Logger::info(LogLevel::high, "RTSP framebuffer stream is dropping frames to keep the render loop responsive");
			}
			m_frames.emplace_back(std::move(frame));
		}
		m_queueSignal.notify_one();
	}

	void FramebufferStreamer::shutdown()
	{
		if (!m_running)
			return;

		m_stopRequested = true;
		closeClient();
		closeListener();
		m_queueSignal.notify_all();

		if (m_workerThread.joinable())
			m_workerThread.join();

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_frames.clear();
		}

		closeStream();
		m_running = false;
		m_ready = false;
		Logger::info(LogLevel::high, "RTSP framebuffer stream stopped");
	}

	void FramebufferStreamer::workerLoop()
	{
		if (!openListener()) {
			m_streamFailed = true;
			m_running = false;
			return;
		}

		while (!m_stopRequested) {
			if (!waitForClient())
				break;

			while (!m_stopRequested && m_clientPlaying) {
				Frame frame;
				{
					std::unique_lock<std::mutex> lock(m_queueMutex);
					m_queueSignal.wait(lock, [&] { return m_stopRequested || !m_frames.empty() || !m_clientPlaying; });
					if (m_stopRequested || !m_clientPlaying)
						break;
					frame = std::move(m_frames.front());
					m_frames.pop_front();
				}

				if (!encodeFrame(frame)) {
					Logger::info(LogLevel::high, "RTSP framebuffer stream client disconnected or write failed");
					break;
				}
			}

			closeClient();
			m_clientPlaying = false;
			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				m_frames.clear();
			}
		}

		closeListener();
		m_ready = false;
	}

	bool FramebufferStreamer::openEncoder(int32_t width, int32_t height)
	{
		if (m_codecContext && m_streamWidth == width && m_streamHeight == height)
			return true;

		if (m_codecContext)
			Logger::info(LogLevel::high, "RTSP framebuffer stream resized to {}x{}", width, height);

		closeStream();

		const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
		if (!codec)
			codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (!codec) {
			Logger::error("RTSP framebuffer stream could not start: H.264 encoder not found in this FFMPEG build");
			return false;
		}

		m_codecContext = avcodec_alloc_context3(codec);
		if (!m_codecContext) {
			Logger::error("RTSP framebuffer stream could not allocate encoder context");
			return false;
		}

		m_codecContext->codec_id = codec->id;
		m_codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
		m_codecContext->width = width;
		m_codecContext->height = height;
		m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
		m_codecContext->time_base = AVRational{ 1, kStreamFps };
		m_codecContext->framerate = AVRational{ kStreamFps, 1 };
		m_codecContext->bit_rate = kStreamBitrate;
		m_codecContext->gop_size = 10;
		m_codecContext->max_b_frames = 0;
		m_codecContext->flags |= AV_CODEC_FLAG_LOW_DELAY;

		AVDictionary* codecOptions = nullptr;
		av_dict_set(&codecOptions, "preset", "veryfast", 0);
		av_dict_set(&codecOptions, "tune", "zerolatency", 0);
		av_dict_set(&codecOptions, "x264-params", "repeat-headers=1:annexb=1:scenecut=0:sync-lookahead=0:rc-lookahead=0", 0);

		const int rc = avcodec_open2(m_codecContext, codec, &codecOptions);
		av_dict_free(&codecOptions);
		if (rc < 0) {
			logFfmpegError("RTSP framebuffer stream could not open encoder", rc);
			return false;
		}

		m_yuvFrame = av_frame_alloc();
		m_packet = av_packet_alloc();
		if (!m_yuvFrame || !m_packet) {
			Logger::error("RTSP framebuffer stream could not allocate FFMPEG frame buffers");
			return false;
		}

		m_yuvFrame->format = m_codecContext->pix_fmt;
		m_yuvFrame->width = width;
		m_yuvFrame->height = height;

		const int yuvSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);
		m_yuvBuffer = static_cast<uint8_t*>(av_malloc(yuvSize));
		if (!m_yuvBuffer) {
			Logger::error("RTSP framebuffer stream could not allocate image buffers");
			return false;
		}
		av_image_fill_arrays(m_yuvFrame->data, m_yuvFrame->linesize, m_yuvBuffer, AV_PIX_FMT_YUV420P, width, height, 1);
		m_streamWidth = width;
		m_streamHeight = height;
		return true;
	}

	bool FramebufferStreamer::openListener()
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			Logger::error("RTSP framebuffer stream could not initialize Winsock");
			return false;
		}
		m_winsockStarted = true;

		const auto listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (listenSocket == kInvalidSocket) {
			Logger::error("RTSP framebuffer stream could not create TCP socket");
			closeListener();
			return false;
		}

		BOOL reuse = TRUE;
		setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(kRtspPort);
		if (bind(listenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
			Logger::error("RTSP framebuffer stream could not bind TCP port {}", kRtspPort);
			closesocket(listenSocket);
			closeListener();
			return false;
		}

		if (listen(listenSocket, 1) == SOCKET_ERROR) {
			Logger::error("RTSP framebuffer stream could not listen on TCP port {}", kRtspPort);
			closesocket(listenSocket);
			closeListener();
			return false;
		}

		m_listenSocket.store(static_cast<uintptr_t>(listenSocket));
		m_ready = true;
		Logger::info(LogLevel::high, "RTSP framebuffer stream listening on rtsp://localhost:{}", kRtspPort);
		return true;
	}

	bool FramebufferStreamer::waitForClient()
	{
		const auto listenSocket = static_cast<SOCKET>(m_listenSocket.load());
		SOCKET clientSocket = kInvalidSocket;
		while (!m_stopRequested) {
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(listenSocket, &readSet);

			timeval timeout = {};
			timeout.tv_usec = 100 * 1000;

			const auto selected = select(0, &readSet, nullptr, nullptr, &timeout);
			if (selected == SOCKET_ERROR)
				return false;
			if (selected == 0)
				continue;

			clientSocket = accept(listenSocket, nullptr, nullptr);
			if (clientSocket != kInvalidSocket)
				break;
			if (m_stopRequested)
				return false;
		}

		if (clientSocket == kInvalidSocket)
			return false;

		m_clientSocket.store(static_cast<uintptr_t>(clientSocket));
		Logger::info(LogLevel::high, "RTSP framebuffer stream client connected");

		std::string buffer;
		char chunk[4096];
		bool playing = false;
		while (!m_stopRequested && !playing) {
			const auto received = recv(clientSocket, chunk, sizeof(chunk), 0);
			if (received <= 0)
				return false;
			buffer.append(chunk, static_cast<size_t>(received));
			size_t requestEnd = 0;
			while ((requestEnd = buffer.find("\r\n\r\n")) != std::string::npos) {
				const auto request = buffer.substr(0, requestEnd + 4);
				buffer.erase(0, requestEnd + 4);
				if (!handleClientRequest(request, playing))
					return false;
				if (playing)
					break;
			}
		}

		m_clientPlaying = playing;
		return playing;
	}

	bool FramebufferStreamer::handleClientRequest(const std::string& request, bool& playing)
	{
		const int cseq = getCSeq(request);
		const auto firstLineEnd = request.find("\r\n");
		Logger::info(
			LogLevel::high,
			"RTSP framebuffer stream request: {}",
			request.substr(0, firstLineEnd == std::string::npos ? request.size() : firstLineEnd)
		);

		if (request.starts_with("OPTIONS")) {
			return sendRtspResponse(cseq, 200, "OK", "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n");
		}

		if (request.starts_with("DESCRIBE")) {
			const std::string sdp =
				"v=0\r\n"
				"o=- 0 0 IN IP4 127.0.0.1\r\n"
				"s=Phoenix Framebuffer\r\n"
				"c=IN IP4 0.0.0.0\r\n"
				"t=0 0\r\n"
				"a=control:*\r\n"
				"m=video 0 RTP/AVP 96\r\n"
				"a=rtpmap:96 H264/90000\r\n"
				"a=fmtp:96 packetization-mode=1;profile-level-id=42e01f\r\n"
				"a=control:trackID=0\r\n";
			const std::string headers = std::format(
				"Content-Type: application/sdp\r\nContent-Base: rtsp://localhost:{}/\r\n",
				kRtspPort
			);
			return sendRtspResponse(cseq, 200, "OK", headers, sdp);
		}

		if (request.starts_with("SETUP")) {
			const auto transport = getHeaderValue(request, "Transport");
			if (transport.find("RTP/AVP/TCP") == std::string::npos) {
				Logger::info(LogLevel::high, "RTSP framebuffer stream rejecting unsupported transport: {}", transport);
				return sendRtspResponse(cseq, 461, "Unsupported Transport");
			}
			const std::string headers = std::format(
				"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\nSession: {}\r\n",
				m_sessionId
			);
			return sendRtspResponse(cseq, 200, "OK", headers);
		}

		if (request.starts_with("PLAY")) {
			playing = true;
			const std::string headers = std::format("Session: {}\r\nRTP-Info: url=rtsp://localhost:{}/trackID=0;seq={};rtptime=0\r\n", m_sessionId, kRtspPort, m_rtpSequence);
			return sendRtspResponse(cseq, 200, "OK", headers);
		}

		if (request.starts_with("TEARDOWN")) {
			sendRtspResponse(cseq, 200, "OK", std::format("Session: {}\r\n", m_sessionId));
			return false;
		}

		return sendRtspResponse(cseq, 405, "Method Not Allowed");
	}

	bool FramebufferStreamer::sendRtspResponse(int cseq, int statusCode, std::string_view statusText, std::string_view headers, std::string_view body)
	{
		const std::string response = std::format(
			"RTSP/1.0 {} {}\r\nCSeq: {}\r\n{}Content-Length: {}\r\n\r\n{}",
			statusCode,
			statusText,
			cseq,
			headers,
			body.size(),
			body
		);
		return sendAll(static_cast<SOCKET>(m_clientSocket.load()), reinterpret_cast<const uint8_t*>(response.data()), response.size());
	}

	bool FramebufferStreamer::encodeFrame(const Frame& frame)
	{
		if (frame.pixels.empty())
			return true;

		if (!openEncoder(frame.width, frame.height))
			return false;

		if (!m_swsContext || m_swsSourceWidth != frame.width || m_swsSourceHeight != frame.height) {
			if (m_swsContext)
				sws_freeContext(m_swsContext);
			m_swsContext = sws_getContext(frame.width, frame.height, AV_PIX_FMT_RGB24, m_streamWidth, m_streamHeight, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
			m_swsSourceWidth = frame.width;
			m_swsSourceHeight = frame.height;
			if (!m_swsContext) {
				Logger::error("RTSP framebuffer stream could not create pixel conversion context");
				return false;
			}
		}

		const uint8_t* sourceData[1] = { frame.pixels.data() + static_cast<size_t>(frame.height - 1) * static_cast<size_t>(frame.width) * 3 };
		const int sourceLinesize[1] = { -frame.width * 3 };
		sws_scale(m_swsContext, sourceData, sourceLinesize, 0, frame.height, m_yuvFrame->data, m_yuvFrame->linesize);

		m_yuvFrame->pts = frame.pts;

		int rc = avcodec_send_frame(m_codecContext, m_yuvFrame);
		if (rc < 0) {
			logFfmpegError("RTSP framebuffer stream could not send frame to encoder", rc);
			return false;
		}

		while (rc >= 0) {
			rc = avcodec_receive_packet(m_codecContext, m_packet);
			if (rc == AVERROR(EAGAIN) || rc == AVERROR_EOF)
				return true;
			if (rc < 0) {
				logFfmpegError("RTSP framebuffer stream could not receive encoded packet", rc);
				return false;
			}
			const bool sent = sendEncodedPacket(m_packet);
			av_packet_unref(m_packet);
			if (!sent)
				return false;
		}
		return true;
	}

	bool FramebufferStreamer::sendEncodedPacket(const AVPacket* packet)
	{
		const auto timestamp = static_cast<uint32_t>((packet->pts < 0 ? 0 : packet->pts) * (90000 / kStreamFps));
		const uint8_t* data = packet->data;
		const size_t size = static_cast<size_t>(packet->size);

		size_t start = 0;
		size_t codeSize = 0;
		if (!findStartCode(data, size, 0, start, codeSize))
			return sendNalUnit(data, size, timestamp, true);

		while (start < size) {
			const size_t nalStart = start + codeSize;
			size_t nextStart = 0;
			size_t nextCodeSize = 0;
			const bool hasNext = findStartCode(data, size, nalStart, nextStart, nextCodeSize);
			const size_t nalEnd = hasNext ? nextStart : size;
			if (nalEnd > nalStart) {
				const bool marker = !hasNext;
				if (!sendNalUnit(data + nalStart, nalEnd - nalStart, timestamp, marker))
					return false;
			}
			if (!hasNext)
				break;
			start = nextStart;
			codeSize = nextCodeSize;
		}
		return true;
	}

	bool FramebufferStreamer::sendNalUnit(const uint8_t* nal, size_t size, uint32_t timestamp, bool marker)
	{
		if (size <= kMaxRtpPayload)
			return sendRtpPayload(nal, size, timestamp, marker);

		const uint8_t nalHeader = nal[0];
		const uint8_t fuIndicator = (nalHeader & 0xE0) | 28;
		const uint8_t nalType = nalHeader & 0x1F;
		size_t offset = 1;
		bool first = true;
		while (offset < size) {
			const size_t chunk = std::min(kMaxRtpPayload - 2, size - offset);
			std::vector<uint8_t> payload(chunk + 2);
			payload[0] = fuIndicator;
			payload[1] = nalType;
			if (first)
				payload[1] |= 0x80;
			if (offset + chunk >= size)
				payload[1] |= 0x40;
			memcpy(payload.data() + 2, nal + offset, chunk);
			if (!sendRtpPayload(payload.data(), payload.size(), timestamp, marker && (offset + chunk >= size)))
				return false;
			first = false;
			offset += chunk;
		}
		return true;
	}

	bool FramebufferStreamer::sendRtpPayload(const uint8_t* payload, size_t size, uint32_t timestamp, bool marker)
	{
		std::vector<uint8_t> packet(4 + 12 + size);
		packet[0] = '$';
		packet[1] = 0;
		const uint16_t rtpSize = static_cast<uint16_t>(12 + size);
		packet[2] = static_cast<uint8_t>(rtpSize >> 8);
		packet[3] = static_cast<uint8_t>(rtpSize & 0xFF);
		packet[4] = 0x80;
		packet[5] = static_cast<uint8_t>(96 | (marker ? 0x80 : 0));
		packet[6] = static_cast<uint8_t>(m_rtpSequence >> 8);
		packet[7] = static_cast<uint8_t>(m_rtpSequence & 0xFF);
		m_rtpSequence++;
		packet[8] = static_cast<uint8_t>(timestamp >> 24);
		packet[9] = static_cast<uint8_t>((timestamp >> 16) & 0xFF);
		packet[10] = static_cast<uint8_t>((timestamp >> 8) & 0xFF);
		packet[11] = static_cast<uint8_t>(timestamp & 0xFF);
		packet[12] = static_cast<uint8_t>(m_rtpSsrc >> 24);
		packet[13] = static_cast<uint8_t>((m_rtpSsrc >> 16) & 0xFF);
		packet[14] = static_cast<uint8_t>((m_rtpSsrc >> 8) & 0xFF);
		packet[15] = static_cast<uint8_t>(m_rtpSsrc & 0xFF);
		memcpy(packet.data() + 16, payload, size);
		return sendAll(static_cast<SOCKET>(m_clientSocket.load()), packet.data(), packet.size());
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
	}

	void FramebufferStreamer::closeClient()
	{
		const auto socket = static_cast<SOCKET>(m_clientSocket.exchange(static_cast<uintptr_t>(kInvalidSocket)));
		if (socket != kInvalidSocket) {
			::shutdown(socket, SD_BOTH);
			closesocket(socket);
		}
		m_clientPlaying = false;
	}

	void FramebufferStreamer::closeListener()
	{
		const auto socket = static_cast<SOCKET>(m_listenSocket.exchange(static_cast<uintptr_t>(kInvalidSocket)));
		if (socket != kInvalidSocket) {
			closesocket(socket);
		}
		if (m_winsockStarted.exchange(false))
			WSACleanup();
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
