// video.cpp
// Spontz Demogroup

// FFmpeg refence: https://ffmpeg.org/doxygen/trunk
// TODO: proper sync
// TODO: double buffer

#include "video.h"

#include <main.h>


Video::Video(bool bDebug, uint32_t uiDecodingThreadCount, double dPlaybackSpeed)
	:
	m_sFileName_("Video not loaded"),
	m_iVideoStreamIndex_(-1),
	m_dFramerate_(0),
	m_iWidth_(0),
	m_iHeight_(0),
	m_uiTexID_(0),
	m_bLoaded_(false),
	m_pFormatContext_(nullptr),
	m_pAVCodec_(nullptr),
	m_pAVCodecParameters_(nullptr),
	m_pCodecContext_(nullptr),
	m_pFrame_(nullptr),
	m_pGLFrame_(nullptr),
	m_pConvertContext_(nullptr),
	m_pAVPacket_(nullptr),
	m_dIntervalFrame_(0),
	m_dNextFrameTime_(0),
	m_pWorkerThread_(nullptr),
	m_bNewFrame_(false),
	m_dTime_(0.0),
	m_bStopWorkerThread_(false),
	m_bDebug_(bDebug),
	m_uiDecodingThreadCount_(uiDecodingThreadCount),
	m_dPlaybackSpeed_(dPlaybackSpeed)
{
}

Video::~Video()
{
	clearData();
}

void Video::clearData()
{
	if (m_pWorkerThread_) {
		m_bStopWorkerThread_ = true;
		m_pWorkerThread_->join();
		delete m_pWorkerThread_;
	}

	if (m_uiTexID_ != 0) {
		glDeleteTextures(1, &m_uiTexID_);
		m_uiTexID_ = 0;
	}

	if (m_pFormatContext_) {
		avformat_close_input(&m_pFormatContext_);
		avformat_free_context(m_pFormatContext_);
	}

	if (m_pAVPacket_) {
		av_packet_unref(m_pAVPacket_);
		av_packet_free(&m_pAVPacket_);
	}

	if (m_pFrame_) {
		av_frame_unref(m_pFrame_);
		av_frame_free(&m_pFrame_);
	}

	if (m_pGLFrame_) {
		av_frame_unref(m_pGLFrame_);
		av_frame_free(&m_pGLFrame_);
	}

	if (m_pCodecContext_)
		avcodec_free_context(&m_pCodecContext_);
}

double Video::renderInterval() const {
	return m_dIntervalFrame_ / m_dPlaybackSpeed_;
};

bool Video::load(std::string const& sFileName, int32_t iVideoStreamIndex)
{
	m_sFileName_ = sFileName;

	// Delete data, in case video has been already loaded
	clearData();

	m_pFormatContext_ = avformat_alloc_context();
	if (!m_pFormatContext_) {
		LOG->Error("%s: could not allocate memory for AVFormatContext.", __FILE__);
		return false;
	}

	// Open file and read header
	// Codecs are not opened
	if (m_bDebug_)
		LOG->Info(
			LogLevel::LOW,
			"%s: Opening \"%s\" and loading format (container) header.",
			__FILE__,
			sFileName.c_str()
		);

	if (avformat_open_input(&m_pFormatContext_, m_sFileName_.c_str(), nullptr, nullptr) != 0) {
		LOG->Error("%s: Error opening \"%s\".", __FILE__, m_sFileName_.c_str());
		return false;
	}

	// Show info
	LOG->Info(
		LogLevel::LOW,
		"%s: %s, %dms, %dbits/s.",
		__FILE__,
		m_pFormatContext_->iformat->name,
		m_pFormatContext_->duration / (AV_TIME_BASE / 1000),
		m_pFormatContext_->bit_rate
	);

	// Read Packets from AVFormatContext to get stream information
	// avformat_find_stream_info populates m_pFormatContext_->streams (of size equals to pFormatContext->nb_streams)
	if (m_bDebug_)
		LOG->Info(LogLevel::LOW, "%s: Finding stream info from format", __FILE__);
	if (avformat_find_stream_info(m_pFormatContext_, nullptr) < 0) {
		LOG->Error("%s: Could not get the stream info.", __FILE__);
		return false;
	}

	for (unsigned int i = 0; i < m_pFormatContext_->nb_streams; ++i) {
		const auto pAVStream = m_pFormatContext_->streams[i];
		const auto pAVCodecParameters = pAVStream->codecpar;

		if (m_bDebug_) {
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
			if (m_iVideoStreamIndex_ == -1)
				if (iVideoStreamIndex == -1 || iVideoStreamIndex == i) {
					LOG->Info(
						LogLevel::LOW,
						"%s: Using video stream #%d.",
						__FILE__,
						i
					);

					m_iVideoStreamIndex_ = i;
					m_dFramerate_ = av_q2d(pAVStream->avg_frame_rate);
					m_dIntervalFrame_ = 1.0 / m_dFramerate_;
					m_pAVCodec_ = pAVCodec;
					m_pAVCodecParameters_ = pAVCodecParameters;
					m_iWidth_ = pAVCodecParameters->width;
					m_iHeight_ = pAVCodecParameters->height;
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

	m_pCodecContext_ = avcodec_alloc_context3(m_pAVCodec_);
	if (!m_pCodecContext_) {
		LOG->Error("%s: failed to allocated memory for AVCodecContext", __FILE__);
		return false;
	}
	m_pCodecContext_->thread_count = m_uiDecodingThreadCount_;

	// Fill the codec context based on the values from the supplied codec parameters
	if (avcodec_parameters_to_context(m_pCodecContext_, m_pAVCodecParameters_) < 0) {
		LOG->Error("%s: failed to copy codec params to codec context", __FILE__);
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	if (avcodec_open2(m_pCodecContext_, m_pAVCodec_, nullptr) < 0) {
		LOG->Error("%s: failed to open codec through avcodec_open2", __FILE__);
		return false;
	}

	m_pFrame_ = av_frame_alloc();
	m_pGLFrame_ = av_frame_alloc();
	
	// Allocate te data buffer for the glFrame
	const auto iSize = av_image_get_buffer_size(
		AV_PIX_FMT_RGB24,
		m_pCodecContext_->width,
		m_pCodecContext_->height,
		1
	);

	const auto puiInternalBuffer = static_cast<const uint8_t*>(
		av_malloc(iSize * sizeof(uint8_t))
	);

	av_image_fill_arrays(
		m_pGLFrame_->data,
		m_pGLFrame_->linesize,
		puiInternalBuffer,
		AV_PIX_FMT_RGB24,
		m_pCodecContext_->width,
		m_pCodecContext_->height,
		1
	);

	if (!m_pFrame_ || !m_pGLFrame_) {
		LOG->Error("%s: failed to allocated memory for AVFrame", __FILE__);
		return false;
	}

	// Create the convert Context, used by the OpenGLFrame
	m_pConvertContext_ = sws_getContext(
		m_pCodecContext_->width,
		m_pCodecContext_->height,
		m_pCodecContext_->pix_fmt,	// Source
		m_pCodecContext_->width,
		m_pCodecContext_->height,
		AV_PIX_FMT_RGB24,			// Destiny (we change the format only)
		0,							// No interpolation
		nullptr,
		nullptr,
		nullptr
	);

	if (!m_pConvertContext_) {
		LOG->Error("%s: Could not create the convert context for OpenGL", __FILE__);
		return false;
	}

	m_pAVPacket_ = av_packet_alloc();
	if (!m_pAVPacket_) {
		LOG->Error("%s: Video: failed to allocated memory for AVPacket", __FILE__);
		return false;
	}

	// Allocate the OpenGL texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_uiTexID_);
	glBindTexture(GL_TEXTURE_2D, m_uiTexID_);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		m_pAVCodecParameters_->width,
		m_pAVCodecParameters_->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	m_bLoaded_ = true;


	static auto fnDecode = [this]() {
		if (m_dTime_ < m_dNextFrameTime_ - renderInterval() || m_dTime_ > m_dNextFrameTime_ + renderInterval() * 2) {
			LOG->Info(
				LogLevel::HIGH,
				"%s: Seeking %s[stream %d] @ %.4fs [desynced by %.4fs]...",
				__FILE__,
				m_sFileName_.c_str(),
				m_iVideoStreamIndex_,
				m_dTime_,
				m_dTime_ - m_dNextFrameTime_
			);

			seekTime(m_dTime_);
			m_dNextFrameTime_ = 0.0f;
		}
		else if (m_dTime_ < m_dNextFrameTime_)
		{
			return;
		}

		if (m_bDebug_) {
			LOG->Info(
				LogLevel::LOW,
				"%s: Time: %.4fs, Next Frame time: %.4fs",
				__FILE__,
				m_dTime_,
				m_dNextFrameTime_
			);
		}

		// Retrieve new frame
		while (m_dNextFrameTime_ <= m_dTime_)
		{
			// fill the Packet with data from the Stream
			if (av_read_frame(m_pFormatContext_, m_pAVPacket_) >= 0) {
				// if it's the video stream
				if (m_pAVPacket_->stream_index == m_iVideoStreamIndex_) {
					auto response = decodePacket();
					if (response < 0)
						LOG->Error("%s: Packet cannot be decoded", __FILE__);
				}
			}
			else {
				// Loop: Start the video again
				seekTime(0);
				// av_seek_frame(pFormatContext, video_stream_index, 0, 0);
			}

			av_packet_unref(m_pAVPacket_);
		}
	};

	while (!m_bNewFrame_)
		fnDecode();

	m_pWorkerThread_ = new std::thread([&] {
		while (!m_bStopWorkerThread_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // hack
			fnDecode();
		}
		});

	return true;
}

void Video::renderVideo(double dTime)
{
	if (!m_bLoaded_)
		return;

	m_dTime_ = dTime;

	if (m_pCodecContext_ && m_bNewFrame_) {
		glBindTextureUnit(0, m_uiTexID_);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			m_pCodecContext_->width,
			m_pCodecContext_->height,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			m_pGLFrame_->data[0]
		);
		m_bNewFrame_ = false;
	}
}

void Video::bind(GLuint uiTexUnit) const
{
	glBindTextureUnit(uiTexUnit, m_uiTexID_);
}

int64_t Video::seekTime(double dTime) const
{
	const auto iTimeMs = static_cast<int64_t>(dTime * 1000.);
	const auto iFrameNumber = av_rescale(
		iTimeMs,
		m_pFormatContext_->streams[m_iVideoStreamIndex_]->time_base.den,
		m_pFormatContext_->streams[m_iVideoStreamIndex_]->time_base.num
	) / 1000;

	if (av_seek_frame(m_pFormatContext_, m_iVideoStreamIndex_, iFrameNumber, 0) < 0)
		LOG->Error("%s: Could not reach position: %.4fs, frame: %d", __FILE__, dTime, iFrameNumber);

	return iFrameNumber;
}

int32_t Video::decodePacket()
{
	// Supply raw packet data as input to a decoder
	auto iResponse = avcodec_send_packet(m_pCodecContext_, m_pAVPacket_);

	if (iResponse < 0) {
		LOG->Error("%s: decodePacket Error while sending a packet to the decoder", __FILE__); // : %s", av_err2str(response));
		return iResponse;
	}

	while (iResponse >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		iResponse = avcodec_receive_frame(m_pCodecContext_, m_pFrame_);
		if (iResponse == AVERROR(EAGAIN) || iResponse == AVERROR_EOF)
			break;
		else if (iResponse < 0) {
			LOG->Error("%s: Error while receiving a frame from the decoder", __FILE__); // : %s", av_err2str(response));
			return iResponse;
		}

		if (iResponse >= 0) {
			if (m_bDebug_) {
				LOG->Info(
					LogLevel::LOW,
					"%s: Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
					__FILE__,
					m_pCodecContext_->frame_number,
					av_get_picture_type_char(m_pFrame_->pict_type),
					m_pFrame_->pkt_size,
					m_pFrame_->pts,
					m_pFrame_->key_frame,
					m_pFrame_->coded_picture_number
				);
			}

			// Scale the image (pFrame) to the OpenGL image (glFrame), using the Convertex cotext
			sws_scale(
				m_pConvertContext_,
				m_pFrame_->data,
				m_pFrame_->linesize,
				0,
				m_pCodecContext_->height,
				m_pGLFrame_->data,
				m_pGLFrame_->linesize
			);

			m_bNewFrame_ = true;
			//m_dNextFrameTime_ = static_cast<double>(m_pCodecContext_->frame_number) * renderInterval();
			m_dNextFrameTime_ = m_dTime_ + renderInterval();
		}
	}

	return 0;
}

std::string const& Video::getFileName() const
{
	return m_sFileName_;
}

GLuint Video::getTexID() const
{
	return m_uiTexID_;
}

int32_t Video::getWidth() const
{
	return m_iWidth_;
}

int32_t Video::getHeight() const
{
	return m_iHeight_;
}
