// video.cpp
// Spontz Demogroup

#include "video.h"

#include <main.h>

#include <chrono>

const bool kDebug = false;

Video::Video()
	:
	m_pGLFrame_(nullptr),
	m_pCodecContext_(nullptr),
	m_pFormatContext_(nullptr),
	m_pFrame_(nullptr),
	m_pAVPacket_(nullptr),
	m_fileName_("Video not loaded"),
	m_pCodec_(nullptr),
	m_pCodecParameters_(nullptr),
	m_videoStreamIndex_(-1),
	m_pConvertContext_(nullptr),
	m_dFramerate_(0),
	m_width_(0),
	m_height_(0),
	m_dIntervalFrame_(0),
	m_dNextFrameTime_(0),
	m_loaded_(false),
	m_texID_(0)
{
}

Video::~Video()
{
	m_shutdown_ = true;

	m_pThread_->join();
	delete m_pThread_;

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

std::string const& Video::getFileName() const { return m_fileName_; }
GLuint Video::getTexID() const { return m_texID_; }
int Video::getWidth() const { return m_width_; }
int Video::getHeight() const { return m_height_; }

bool Video::load(std::string const& fileName)
{
	m_fileName_ = fileName;
	// Allocate memory for the Context: http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
	m_pFormatContext_ = avformat_alloc_context();

	if (!m_pFormatContext_) {
		LOG->Error("Video: could not allocate memory for Format Context");
		return false;
	}

	if (kDebug)
		LOG->Info(LogLevel::LOW, "Video: Opening the input file (%s) and loading format (container) header", fileName.c_str());

	// Open the file and read its header.
	// The codecs are not opened:
	// http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
	if (avformat_open_input(&m_pFormatContext_, m_fileName_.c_str(), nullptr, nullptr) != 0) {
		LOG->Error("Video: could not open the file %s", m_fileName_.c_str());
		return false;
	}

	// Show info file
	LOG->Info(
		LogLevel::LOW,
		"Video: Format %s, duration %lld us, bit_rate %lld",
		m_pFormatContext_->iformat->name,
		m_pFormatContext_->duration,
		m_pFormatContext_->bit_rate
	);

	// LOG->Info(LogLevel::LOW, "Video: Finding stream info from format");
	// Read Packets from the Format to get stream information
	// this function populates pFormatContext->streams (of size equals to pFormatContext->nb_streams)
	// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
	if (avformat_find_stream_info(m_pFormatContext_, nullptr) < 0) {
		LOG->Error("Video: could not get the stream info");
		return false;
	}

	// loop though all the streams and print its main information
	for (unsigned int i = 0; i < m_pFormatContext_->nb_streams; ++i) {
		const auto pLocalCodecParameters = m_pFormatContext_->streams[i]->codecpar;

		if (kDebug) {
			LOG->Info(
				LogLevel::LOW,
				"Video: AVStream->time_base before open coded %d/%d",
				m_pFormatContext_->streams[i]->time_base.num,
				m_pFormatContext_->streams[i]->time_base.den
			);
			LOG->Info(
				LogLevel::LOW,
				"Video: AVStream->r_frame_rate before open coded %d/%d",
				m_pFormatContext_->streams[i]->r_frame_rate.num,
				m_pFormatContext_->streams[i]->r_frame_rate.den
			);
			LOG->Info(
				LogLevel::LOW,
				"Video: AVStream->start_time %" PRId64,
				m_pFormatContext_->streams[i]->start_time
			);
			LOG->Info(
				LogLevel::LOW,
				"Video: AVStream->duration %" PRId64,
				m_pFormatContext_->streams[i]->duration
			);
			LOG->Info(LogLevel::LOW, "Video: finding the proper decoder (CODEC)");
		}

		// finds the registered decoder for a codec ID
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
		const auto pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

		if (pLocalCodec == nullptr) {
			LOG->Error("Video: unsupported codec!");
			return false;
		}

		// when the stream is a video we store its index, codec parameters and codec
		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (m_videoStreamIndex_ == -1) {
				m_videoStreamIndex_ = i;
				m_dFramerate_ = av_q2d(m_pFormatContext_->streams[i]->avg_frame_rate);
				m_dIntervalFrame_ = 1.0 / m_dFramerate_;
				m_pCodec_ = pLocalCodec;
				m_pCodecParameters_ = pLocalCodecParameters;
				m_width_ = pLocalCodecParameters->width;
				m_height_ = pLocalCodecParameters->height;
			}
			LOG->Info(
				LogLevel::LOW,
				"Video Codec: resolution %dx%d",
				pLocalCodecParameters->width,
				pLocalCodecParameters->height
			);
		}
		else {
			if (kDebug) {
				// There is no need to play audio
				if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO)
					LOG->Info(
						LogLevel::LOW,
						"Audio Codec: %d channels, sample rate %d",
						pLocalCodecParameters->channels,
						pLocalCodecParameters->sample_rate
					);
			}
		}

		// print its name, id and bitrate
		LOG->Info(
			LogLevel::LOW,
			"Video: Codec %s ID %d bit_rate %lld",
			pLocalCodec->name,
			pLocalCodec->id,
			m_pCodecParameters_->bit_rate
		);
	}

	// https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
	m_pCodecContext_ = avcodec_alloc_context3(m_pCodec_);
	if (!m_pCodecContext_) {
		LOG->Error("Video: failed to allocated memory for AVCodecContext");
		return false;
	}

	m_pCodecContext_->thread_count = 10; // hack

	// Fill the codec context based on the values from the supplied codec parameters
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
	if (avcodec_parameters_to_context(m_pCodecContext_, m_pCodecParameters_) < 0) {
		LOG->Error("Video: failed to copy codec params to codec context");
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
	if (avcodec_open2(m_pCodecContext_, m_pCodec_, nullptr) < 0) {
		LOG->Error("Video: failed to open codec through avcodec_open2");
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
		LOG->Error("Video: failed to allocated memory for AVFrame");
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
		LOG->Error("Could not create the convert context for OpenGL");
		return false;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVPacket.html
	m_pAVPacket_ = av_packet_alloc();
	if (!m_pAVPacket_) {
		LOG->Error("Video: failed to allocated memory for AVPacket");
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
		m_pCodecParameters_->width,
		m_pCodecParameters_->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	m_loaded_ = true;
	return true;
}



void Video::renderVideo(double dTime)
{
	if (!m_loaded_)
		return;

	m_dTime_ = dTime;

	static auto funcDecode = [this](){

		if (m_dTime_ < m_dNextFrameTime_ - m_dIntervalFrame_ || m_dTime_ > m_dNextFrameTime_ + m_dIntervalFrame_) {
			LOG->Info(LogLevel::LOW, "Seek needed!");
			OutputDebugString(TEXT("Seek\n"));
			// seekTime(dTime); // hack
			m_dNextFrameTime_ = m_dTime_;
		}
		else if (m_dTime_ < m_dNextFrameTime_) {
			return;
		}

		if (kDebug) {
			LOG->Info(
				LogLevel::LOW,
				"Time: %.4f, Next Frame time: %.4f",
				m_dTime_,
				m_dNextFrameTime_
			);
		}

		// Retrieve new frame
		m_dNextFrameTime_ += m_dIntervalFrame_;
		int response = 0;

		{
			const std::lock_guard _(m_mutex_);

			// fill the Packet with data from the Stream
			// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
			if (av_read_frame(m_pFormatContext_, m_pAVPacket_) >= 0) {
				// if it's the video stream
				if (m_pAVPacket_->stream_index == m_videoStreamIndex_) {
					response = decodePacket();
					if (response < 0)
						LOG->Error("Video: Packet cannot be decoded");
					else
						m_newFrame_ = true;
				}
			}
			else {
				// Loop: Start the video again
				OutputDebugString(TEXT("Seek (0)\n"));
				seekTime(0); //av_seek_frame(pFormatContext, video_stream_index, 0, 0);
			}
			// https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
			av_packet_unref(m_pAVPacket_);
		}
		};

	if (m_pThread_ == nullptr)
		m_pThread_ = new std::thread([&] {

			while (!m_shutdown_) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1)); // hack
				funcDecode();
			}

			});

	if (m_pCodecContext_) {
		if (m_newFrame_) {
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
			m_newFrame_ = false;
		}
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
		LOG->Error("Video: Could not reach position: %f, frame: %d", dTime, iFrameNumber);
}

int Video::decodePacket()
{
	// Supply raw packet data as input to a decoder
	// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
	int response = avcodec_send_packet(m_pCodecContext_, m_pAVPacket_);

	if (response < 0) {
		LOG->Error("Video::decodePacket Error while sending a packet to the decoder"); // : %s", av_err2str(response));
		return response;
	}

	while (response >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
		response = avcodec_receive_frame(m_pCodecContext_, m_pFrame_);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		}
		else if (response < 0) {
			LOG->Error("Error while receiving a frame from the decoder"); // : %s", av_err2str(response));
			return response;
		}

		if (response >= 0) {
			if (kDebug) {
				LOG->Info(
					LogLevel::LOW,
					"Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
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
		}
	}

	return 0;
}
