// video.cpp
// Spontz Demogroup

// FFmpeg refence: https://ffmpeg.org/doxygen/trunk
// TODO: proper sync
// TODO: double buffer

#include "video.h"

#include <main.h>

#include <chrono>


Video::Video(
	bool debug,
	uint32_t decodingThreadCount, // ideally logical cores - 1
	double playbackSpeed // 1.0 means normal speed, 2.0 double speed, etc. 
)
	:
	m_debug_(debug),
	m_decodingThreadCount_(decodingThreadCount),
	m_playbackSpeed_(playbackSpeed),
	m_fileName_("Video not loaded"),
	m_videoStreamIndex_(-1),
	m_loaded_(false),
	m_stopWorkerThread_(false),
	m_bNewFrame_(false),
	m_dTime_(0.0),
	m_dFramerate_(0),
	m_width_(0),
	m_height_(0),
	m_dIntervalFrame_(0),
	m_dNextFrameTime_(0),
	m_texID_(0),
	m_pGLFrame_(nullptr),
	m_pCodecContext_(nullptr),
	m_pFormatContext_(nullptr),
	m_pFrame_(nullptr),
	m_pAVPacket_(nullptr),
	m_pAVCodec_(nullptr),
	m_pAVCodecParameters_(nullptr),
	m_pConvertContext_(nullptr),
	m_pWorkerThread_(nullptr)
{
}

Video::~Video()
{
	if (m_pWorkerThread_) {
		m_stopWorkerThread_ = true;
		m_pWorkerThread_->join();
		delete m_pWorkerThread_;
	}

	if (m_texID_ != 0) {
		glDeleteTextures(1, &m_texID_);
		m_texID_ = 0;
	}

	avformat_close_input(&m_pFormatContext_);
	avformat_free_context(m_pFormatContext_);

	av_packet_unref(m_pAVPacket_);
	av_packet_free(&m_pAVPacket_);

	av_frame_unref(m_pFrame_);
	av_frame_unref(m_pGLFrame_);
	av_frame_free(&m_pFrame_);
	av_frame_free(&m_pGLFrame_);

	avcodec_free_context(&m_pCodecContext_);
}


double Video::renderInterval() const {
	return m_dIntervalFrame_ / m_playbackSpeed_;
};

bool Video::load(std::string const& fileName, int videoStreamIndex)
{
	m_fileName_ = fileName;

	m_pFormatContext_ = avformat_alloc_context();
	if (!m_pFormatContext_) {
		LOG->Error("%s: could not allocate memory for AVFormatContext.", __FILE__);
		return false;
	}

	// Open file and read header
	// Codecs are not opened
	if (m_debug_)
		LOG->Info(LogLevel::LOW, "%s: Opening \"%s\" and loading format (container) header.", __FILE__, fileName.c_str());
	if (avformat_open_input(&m_pFormatContext_, m_fileName_.c_str(), nullptr, nullptr) != 0) {
		LOG->Error("%s: Error opening \"%s\".", __FILE__, m_fileName_.c_str());
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
	if (m_debug_)
		LOG->Info(LogLevel::LOW, "%s: Finding stream info from format", __FILE__);
	if (avformat_find_stream_info(m_pFormatContext_, nullptr) < 0) {
		LOG->Error("%s: Could not get the stream info.", __FILE__);
		return false;
	}

	for (unsigned int i = 0; i < m_pFormatContext_->nb_streams; ++i) {
		const auto pAVStream = m_pFormatContext_->streams[i];
		const auto pAVCodecParameters = pAVStream->codecpar;

		if (m_debug_) {
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
			if (m_videoStreamIndex_ == -1)
				if (videoStreamIndex == -1 || videoStreamIndex == i) {
					LOG->Info(
						LogLevel::LOW,
						"%s: Using video stream #%d.",
						__FILE__,
						i
					);

					m_videoStreamIndex_ = i;
					m_dFramerate_ = av_q2d(pAVStream->avg_frame_rate);
					m_dIntervalFrame_ = 1.0 / m_dFramerate_;
					m_pAVCodec_ = pAVCodec;
					m_pAVCodecParameters_ = pAVCodecParameters;
					m_width_ = pAVCodecParameters->width;
					m_height_ = pAVCodecParameters->height;
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
	m_pCodecContext_->thread_count = m_decodingThreadCount_;

	// Fill the codec context based on the values from the supplied codec parameters
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
	if (avcodec_parameters_to_context(m_pCodecContext_, m_pAVCodecParameters_) < 0) {
		LOG->Error("%s: failed to copy codec params to codec context", __FILE__);
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
	if (avcodec_open2(m_pCodecContext_, m_pAVCodec_, nullptr) < 0) {
		LOG->Error("%s: failed to open codec through avcodec_open2", __FILE__);
		return false;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVFrame.html
	m_pFrame_ = av_frame_alloc();
	m_pGLFrame_ = av_frame_alloc();
	// Allocate te data buffer for the glFrame
	const int size = av_image_get_buffer_size(
		AV_PIX_FMT_RGB24,
		m_pCodecContext_->width,
		m_pCodecContext_->height,
		1
	);
	uint8_t* internal_buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
	av_image_fill_arrays(
		(uint8_t**)((AVPicture*)m_pGLFrame_->data),
		(int*)((AVPicture*)m_pGLFrame_->linesize),
		internal_buffer,
		AV_PIX_FMT_RGB24,
		m_pCodecContext_->width,
		m_pCodecContext_->height,
		1
	);
	if ((!m_pFrame_) || (!m_pGLFrame_)) {
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

	// https://ffmpeg.org/doxygen/trunk/structAVPacket.html
	m_pAVPacket_ = av_packet_alloc();
	if (!m_pAVPacket_) {
		LOG->Error("%s: Video: failed to allocated memory for AVPacket", __FILE__);
		return false;
	}

	// Allocate the OpenGL texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_texID_);
	glBindTexture(GL_TEXTURE_2D, m_texID_);
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

	m_loaded_ = true;


	static auto funcDecode = [this]() {

		if (m_dTime_ < m_dNextFrameTime_)
			return;

		if (m_dTime_ < m_dNextFrameTime_ - renderInterval() || m_dTime_ > m_dNextFrameTime_ + renderInterval()) {
			LOG->Info(LogLevel::MED, "%s: WARNING: FFmpeg video stream seek needed.", __FILE__);

			//const auto a = std::chrono::high_resolution_clock::now();
			seekTime(m_dTime_); // hack
			//const auto b = std::chrono::high_resolution_clock::now();

			m_dNextFrameTime_ = static_cast<double>(m_pCodecContext_->frame_number) * renderInterval();
		}

		/*
		if (m_bNewFrame_)
			return;
		*/

		if (m_debug_) {
			LOG->Info(
				LogLevel::LOW,
				"%s: Time: %.4f, Next Frame time: %.4f",
				__FILE__,
				m_dTime_,
				m_dNextFrameTime_
			);
		}

		// Retrieve new frame
		int response = 0;

		while (m_dNextFrameTime_ <= m_dTime_)
		{
			// const std::lock_guard _(m_mutex_);

			// fill the Packet with data from the Stream
			// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
			if (av_read_frame(m_pFormatContext_, m_pAVPacket_) >= 0) {
				// if it's the video stream
				if (m_pAVPacket_->stream_index == m_videoStreamIndex_) {
					response = decodePacket();
					if (response < 0)
						LOG->Error("%s: Packet cannot be decoded", __FILE__);
				}
			}
			else {
				// Loop: Start the video again
				// OutputDebugString(TEXT("Seek (0)\n"));
				seekTime(0); //av_seek_frame(pFormatContext, video_stream_index, 0, 0);
			}
			// https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
			av_packet_unref(m_pAVPacket_);
		}
	};

	while (!m_bNewFrame_)
		funcDecode();

	m_pWorkerThread_ = new std::thread([&] {
		while (!m_stopWorkerThread_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // hack
			funcDecode();
		}
		});

	return true;
}

void Video::renderVideo(double dTime)
{
	if (!m_loaded_)
		return;

	m_dTime_ = dTime;

	if (m_pCodecContext_ && m_bNewFrame_) {
		glBindTextureUnit(0, m_texID_);
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

void Video::bind(int texUnit) const
{
	glBindTextureUnit(texUnit, m_texID_);
}

void Video::seekTime(double dTime)
{
	const auto iTimeMs = static_cast<int64_t>(dTime * 1000.);
	const auto iFrameNumber = av_rescale(
		iTimeMs,
		m_pFormatContext_->streams[m_videoStreamIndex_]->time_base.den,
		m_pFormatContext_->streams[m_videoStreamIndex_]->time_base.num
	) / 1000;

	if (av_seek_frame(m_pFormatContext_, m_videoStreamIndex_, iFrameNumber, 0) < 0)
		LOG->Error("%s: Could not reach position: %f, frame: %d", __FILE__, dTime, iFrameNumber);
}

int Video::decodePacket()
{
	// Supply raw packet data as input to a decoder
	// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
	int response = avcodec_send_packet(m_pCodecContext_, m_pAVPacket_);

	if (response < 0) {
		LOG->Error("%s: decodePacket Error while sending a packet to the decoder", __FILE__); // : %s", av_err2str(response));
		return response;
	}

	while (response >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
		response = avcodec_receive_frame(m_pCodecContext_, m_pFrame_);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
			break;
		else if (response < 0) {
			LOG->Error("%s: Error while receiving a frame from the decoder", __FILE__); // : %s", av_err2str(response));
			return response;
		}

		if (response >= 0) {
			if (m_debug_) {
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
			m_dNextFrameTime_ = static_cast<double>(m_pCodecContext_->frame_number) * renderInterval();
		}
	}

	return 0;
}

std::string const& Video::getFileName() const
{
	return m_fileName_;
}

GLuint Video::getTexID() const
{
	return m_texID_;
}

int Video::getWidth() const
{
	return m_width_;
}

int Video::getHeight() const
{
	return m_height_;
}
