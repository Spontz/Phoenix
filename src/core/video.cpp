// video.cpp
// Spontz Demogroup

// FFmpeg refence: https://ffmpeg.org/doxygen/trunk
// TODO: proper sync
// TODO: double buffer

#include "video.h"

#include <main.h>


Video::Video(bool bDebug)
	:
	m_dFramerate(0.),
	m_iWidth(0),
	m_iHeight(0),
	m_uiTextureOGLName(0),
	m_bLoaded(false),
	m_pFormatContext(nullptr),
	m_pAVCodec(nullptr),
	m_pAVCodecParameters(nullptr),
	m_pCodecContext(nullptr),
	m_pFrame(nullptr),
	m_pGLFrame(nullptr),
	m_pConvertContext(nullptr),
	m_pAVPacket(nullptr),
	m_dIntervalFrame(0),
	m_dNextFrameTime(0),
	m_pWorkerThread(nullptr),
	m_bNewFrame(false),
	m_dTime(0.),
	m_bStopWorkerThread(false),
	m_bDebug(bDebug)
{
}

Video::~Video()
{
	clearData();
}

void Video::clearData()
{
	if (m_pWorkerThread) {
		m_bStopWorkerThread = true;
		m_pWorkerThread->join();
		delete m_pWorkerThread;
	}

	if (m_uiTextureOGLName != 0) {
		glDeleteTextures(1, &m_uiTextureOGLName);
		m_uiTextureOGLName = 0;
	}

	if (m_pFormatContext) {
		avformat_close_input(&m_pFormatContext);
		avformat_free_context(m_pFormatContext);
	}

	if (m_pAVPacket) {
		av_packet_unref(m_pAVPacket);
		av_packet_free(&m_pAVPacket);
	}

	if (m_pFrame) {
		av_frame_unref(m_pFrame);
		av_frame_free(&m_pFrame);
	}

	if (m_pGLFrame) {
		av_frame_unref(m_pGLFrame);
		av_frame_free(&m_pGLFrame);
	}

	if (m_pCodecContext)
		avcodec_free_context(&m_pCodecContext);
}

double Video::renderInterval() const
{
	return m_dIntervalFrame / m_VideoSource.m_dPlaybackSpeed;
};

bool Video::load(CVideoSource const& videoSource)
{
	m_VideoSource = videoSource;

	// Delete data, in case video has been already loaded
	clearData();

	m_pFormatContext = avformat_alloc_context();
	if (!m_pFormatContext) {
		LOG->Error("%s: could not allocate memory for AVFormatContext.", __FILE__);
		return false;
	}

	// Open file and read header
	// Codecs are not opened
	if (m_bDebug)
		LOG->Info(
			LogLevel::LOW,
			"%s: Opening \"%s\" and loading format (container) header.",
			__FILE__,
			m_VideoSource.m_sPath.c_str()
		);

	if (avformat_open_input(&m_pFormatContext, m_VideoSource.m_sPath.c_str(), nullptr, nullptr) != 0) {
		LOG->Error("%s: Error opening \"%s\".", __FILE__, m_VideoSource.m_sPath.c_str());
		return false;
	}

	// Show info
	LOG->Info(
		LogLevel::LOW,
		"%s: %s, %dms, %dbits/s.",
		__FILE__,
		m_pFormatContext->iformat->name,
		m_pFormatContext->duration / (AV_TIME_BASE / 1000),
		m_pFormatContext->bit_rate
	);

	// Read Packets from AVFormatContext to get stream information
	// avformat_find_streainfo populates pFormatContext_->streams (of size equals to pFormatContext->nb_streams)
	if (m_bDebug)
		LOG->Info(LogLevel::LOW, "%s: Finding stream info from format", __FILE__);
	if (avformat_find_stream_info(m_pFormatContext, nullptr) < 0) {
		LOG->Error("%s: Could not get the stream info.", __FILE__);
		return false;
	}

	for (unsigned int i = 0; i < m_pFormatContext->nb_streams; ++i) {
		const auto pAVStream = m_pFormatContext->streams[i];
		const auto pAVCodecParameters = pAVStream->codecpar;

		if (m_bDebug) {
			LOG->Info(
				LogLevel::LOW,
				"%s: AVStream->time_base before open coded %d/%d",
				__FILE__,
				pAVStream->time_base.num,
				pAVStream->time_base.den
			);
			LOG->Info(
				LogLevel::LOW,
				"%s: AVStream->r_frame_rate before open coded %d/%d",
				__FILE__,
				pAVStream->r_frame_rate.num,
				pAVStream->r_frame_rate.den
			);
			LOG->Info(
				LogLevel::LOW,
				"%s: AVStream->start_time %lld",
				__FILE__,
				pAVStream->start_time
			);
			LOG->Info(
				LogLevel::LOW,
				"%s: AVStream->duration %lld",
				__FILE__,
				pAVStream->duration
			);
			LOG->Info(LogLevel::LOW, "%s: finding the proper decoder (CODEC)", __FILE__);
		}

		// Find AVCodec given an AVCodecID
		const auto pAVCodec = avcodec_find_decoder(pAVCodecParameters->codec_id);
		if (pAVCodec == nullptr) {
			LOG->Error("%s: Unsupported codec.", __FILE__);
			return false;
		}

		switch (pAVCodecParameters->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			// Show video stream info
			LOG->Info(
				LogLevel::LOW,
				"%s: #%d Video stream[CodecID:%d], %s, %d channels, %dx%d, %dhz, %dbps.",
				__FILE__,
				i,
				pAVCodec->id,
				pAVCodec->name,
				pAVCodecParameters->channels,
				pAVCodecParameters->width,
				pAVCodecParameters->height,
				pAVCodecParameters->sample_rate,
				pAVCodecParameters->bit_rate
			);

			// Store video stream index, codec parameters and codec
			if (m_VideoSource.m_iVideoStreamIndex == -1)
				if (videoSource.m_iVideoStreamIndex == -1 || videoSource.m_iVideoStreamIndex == i) {
					LOG->Info(
						LogLevel::LOW,
						"%s: Using video stream #%d.",
						__FILE__,
						i
					);

					m_VideoSource.m_iVideoStreamIndex = i;
					m_dFramerate = av_q2d(pAVStream->avg_frame_rate);
					m_dIntervalFrame = 1.0 / m_dFramerate;
					m_pAVCodec = pAVCodec;
					m_pAVCodecParameters = pAVCodecParameters;
					m_iWidth = pAVCodecParameters->width;
					m_iHeight = pAVCodecParameters->height;
				}

			break;

		default:
			// Show extra stream info
			LOG->Info(
				LogLevel::LOW,
				"%s: #%d %s stream[CodecID:%d], %s, %d channels, %dhz, %d, %dbps.",
				__FILE__,
				i,
				pAVCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO ? "Audio" : "Extra",
				pAVCodec->id,
				pAVCodec->name,
				pAVCodecParameters->channels,
				pAVCodecParameters->sample_rate,
				pAVCodecParameters->bit_rate
			);

			break;
		}
	}

	m_pCodecContext = avcodec_alloc_context3(m_pAVCodec);
	if (!m_pCodecContext) {
		LOG->Error("%s: failed to allocated memory for AVCodecContext", __FILE__);
		return false;
	}
	m_pCodecContext->thread_count = m_VideoSource.m_uiDecodingThreadCount;

	// Fill the codec context based on the values from the supplied codec parameters
	if (avcodec_parameters_to_context(m_pCodecContext, m_pAVCodecParameters) < 0) {
		LOG->Error("%s: failed to copy codec params to codec context", __FILE__);
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	if (avcodec_open2(m_pCodecContext, m_pAVCodec, nullptr) < 0) {
		LOG->Error("%s: failed to open codec through avcodec_open2", __FILE__);
		return false;
	}

	m_pFrame = av_frame_alloc();
	m_pGLFrame = av_frame_alloc();
	
	// Allocate te data buffer for the glFrame
	const auto iSize = av_image_get_buffer_size(
		AV_PIX_FMT_RGB24,
		m_pCodecContext->width,
		m_pCodecContext->height,
		1
	);

	const auto puiInternalBuffer = static_cast<const uint8_t*>(
		av_malloc(iSize * sizeof(uint8_t))
	);

	av_image_fill_arrays(
		m_pGLFrame->data,
		m_pGLFrame->linesize,
		puiInternalBuffer,
		AV_PIX_FMT_RGB24,
		m_pCodecContext->width,
		m_pCodecContext->height,
		1
	);

	if (!m_pFrame || !m_pGLFrame) {
		LOG->Error("%s: failed to allocated memory for AVFrame", __FILE__);
		return false;
	}

	// Create the convert Context, used by the OpenGLFrame
	m_pConvertContext = sws_getContext(
		m_pCodecContext->width,
		m_pCodecContext->height,
		m_pCodecContext->pix_fmt,	// Source
		m_pCodecContext->width,
		m_pCodecContext->height,
		AV_PIX_FMT_RGB24,			// Destiny (we change the format only)
		0,							// No interpolation
		nullptr,
		nullptr,
		nullptr
	);

	if (!m_pConvertContext) {
		LOG->Error("%s: Could not create the convert context for OpenGL", __FILE__);
		return false;
	}

	m_pAVPacket = av_packet_alloc();
	if (!m_pAVPacket) {
		LOG->Error("%s: Video: failed to allocated memory for AVPacket", __FILE__);
		return false;
	}

	// Allocate the OpenGL texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_uiTextureOGLName);
	glBindTexture(GL_TEXTURE_2D, m_uiTextureOGLName);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		m_pAVCodecParameters->width,
		m_pAVCodecParameters->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	m_bLoaded = true;

	while (!m_bNewFrame)
		decode();

	m_pWorkerThread = new std::thread([&] {
		while (!m_bStopWorkerThread) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // hack
			decode();
		}
		});

	return true;
}

void Video::decode()
{
	if (
		m_dTime < m_dNextFrameTime - renderInterval() ||
		m_dTime > m_dNextFrameTime + renderInterval() * 2
		) {
		LOG->Info(
			LogLevel::HIGH,
			"%s: Seeking %s[stream %d] @ %.4fs [desynced by %.4fs]...",
			__FILE__,
			m_VideoSource.m_sPath.c_str(),
			m_VideoSource.m_iVideoStreamIndex,
			m_dTime,
			m_dTime - m_dNextFrameTime
		);

		seekTime(m_dTime);
		m_dNextFrameTime = 0.0f;
	}
	else if (m_dTime < m_dNextFrameTime)
	{
		return;
	}

	if (m_bDebug) {
		LOG->Info(
			LogLevel::LOW,
			"%s: Time: %.4fs, Next Frame time: %.4fs",
			__FILE__,
			m_dTime,
			m_dNextFrameTime
		);
	}

	// Retrieve new frame
	while (m_dNextFrameTime <= m_dTime)
	{
		// fill the Packet with data from the Stream
		if (av_read_frame(m_pFormatContext, m_pAVPacket) >= 0) {
			// if it's the video stream
			if (m_pAVPacket->stream_index == m_VideoSource.m_iVideoStreamIndex) {
				if (decodePacket() < 0)
					LOG->Error("%s: Packet cannot be decoded", __FILE__);
			}
		}
		else {
			// Loop: Start the video again
			seekTime(0);
			// av_seek_frame(pFormatContext, video_streaindex, 0, 0);
		}

		av_packet_unref(m_pAVPacket);
	}
}

void Video::renderVideo(double dTime)
{
	m_dTime = dTime;

	if (!m_bLoaded)
		return;
		
	if (m_pCodecContext && m_bNewFrame) {
		glBindTextureUnit(0, m_uiTextureOGLName);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			m_pCodecContext->width,
			m_pCodecContext->height,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			m_pGLFrame->data[0]
		);
		m_bNewFrame = false;
	}
}

void Video::bind(GLuint uiTexUnit) const
{
	glBindTextureUnit(uiTexUnit, m_uiTextureOGLName);
}

int64_t Video::seekTime(double dSeconds) const
{
	const auto iTimeMs = static_cast<int64_t>(dSeconds * 1000.);
	const auto iFrameNumber = av_rescale(
		iTimeMs,
		m_pFormatContext->streams[m_VideoSource.m_iVideoStreamIndex]->time_base.den,
		m_pFormatContext->streams[m_VideoSource.m_iVideoStreamIndex]->time_base.num
	) / 1000;

	if (av_seek_frame(m_pFormatContext, m_VideoSource.m_iVideoStreamIndex, iFrameNumber, 0) < 0)
		LOG->Error("%s: Could not reach position: %.4fs, frame: %d", __FILE__, dSeconds, iFrameNumber);

	return iFrameNumber;
}

int32_t Video::decodePacket()
{
	// Supply raw packet data as input to a decoder
	auto iResponse = avcodec_send_packet(m_pCodecContext, m_pAVPacket);

	if (iResponse < 0) {
		LOG->Error("%s: decodePacket Error while sending a packet to the decoder", __FILE__); // : %s", av_err2str(response));
		return iResponse;
	}

	while (iResponse >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		iResponse = avcodec_receive_frame(m_pCodecContext, m_pFrame);
		if (iResponse == AVERROR(EAGAIN) || iResponse == AVERROR_EOF)
			break;
		else if (iResponse < 0) {
			LOG->Error("%s: Error while receiving a frame from the decoder", __FILE__); // : %s", av_err2str(response));
			return iResponse;
		}

		if (iResponse >= 0) {
			if (m_bDebug) {
				LOG->Info(
					LogLevel::LOW,
					"%s: Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
					__FILE__,
					m_pCodecContext->frame_number,
					av_get_picture_type_char(m_pFrame->pict_type),
					m_pFrame->pkt_size,
					m_pFrame->pts,
					m_pFrame->key_frame,
					m_pFrame->coded_picture_number
				);
			}

			// Scale the image (pFrame) to the OpenGL image (glFrame), using the Convertex cotext
			sws_scale(
				m_pConvertContext,
				m_pFrame->data,
				m_pFrame->linesize,
				0,
				m_pCodecContext->height,
				m_pGLFrame->data,
				m_pGLFrame->linesize
			);

			m_bNewFrame = true;
			//dNextFrameTime_ = static_cast<double>(pCodecContext_->frame_number) * renderInterval();
			m_dNextFrameTime = m_dTime + renderInterval();
		}
	}

	return 0;
}

std::string const& Video::getFileName() const
{
	return m_VideoSource.m_sPath;
}

GLuint Video::getTexID() const
{
	return m_uiTextureOGLName;
}

int32_t Video::getWidth() const
{
	return m_iWidth;
}

int32_t Video::getHeight() const
{
	return m_iHeight;
}
