// video.cpp
// Spontz Demogroup

// FFmpeg refence: https://ffmpeg.org/doxygen/trunk
// TODO: proper sync
// TODO: double buffer

#include "video.h"

#include <main.h>


Video::Video(bool bDebug)
	:
	dFramerate_(0.),
	iWidth_(0),
	iHeight_(0),
	uiTextureOGLName_(0),
	bLoaded_(false),
	pFormatContext_(nullptr),
	pAVCodec_(nullptr),
	pAVCodecParameters_(nullptr),
	pCodecContext_(nullptr),
	pFrame_(nullptr),
	pGLFrame_(nullptr),
	pConvertContext_(nullptr),
	pAVPacket_(nullptr),
	dIntervalFrame_(0),
	dNextFrameTime_(0),
	pWorkerThread_(nullptr),
	bNewFrame_(false),
	dTime_(0.),
	bStopWorkerThread_(false),
	bDebug_(bDebug)
{
}

Video::~Video()
{
	clearData();
}

void Video::clearData()
{
	if (pWorkerThread_) {
		bStopWorkerThread_ = true;
		pWorkerThread_->join();
		delete pWorkerThread_;
	}

	if (uiTextureOGLName_ != 0) {
		glDeleteTextures(1, &uiTextureOGLName_);
		uiTextureOGLName_ = 0;
	}

	if (pFormatContext_) {
		avformat_close_input(&pFormatContext_);
		avformat_free_context(pFormatContext_);
	}

	if (pAVPacket_) {
		av_packet_unref(pAVPacket_);
		av_packet_free(&pAVPacket_);
	}

	if (pFrame_) {
		av_frame_unref(pFrame_);
		av_frame_free(&pFrame_);
	}

	if (pGLFrame_) {
		av_frame_unref(pGLFrame_);
		av_frame_free(&pGLFrame_);
	}

	if (pCodecContext_)
		avcodec_free_context(&pCodecContext_);
}

double Video::renderInterval() const
{
	return dIntervalFrame_ / VideoSource_.dPlaybackSpeed_;
};

bool Video::load(CVideoSource const& videoSource)
{
	VideoSource_ = videoSource;

	// Delete data, in case video has been already loaded
	clearData();

	pFormatContext_ = avformat_alloc_context();
	if (!pFormatContext_) {
		LOG->Error("%s: could not allocate memory for AVFormatContext.", __FILE__);
		return false;
	}

	// Open file and read header
	// Codecs are not opened
	if (bDebug_)
		LOG->Info(
			LogLevel::LOW,
			"%s: Opening \"%s\" and loading format (container) header.",
			__FILE__,
			VideoSource_.sPath_.c_str()
		);

	if (avformat_open_input(&pFormatContext_, VideoSource_.sPath_.c_str(), nullptr, nullptr) != 0) {
		LOG->Error("%s: Error opening \"%s\".", __FILE__, VideoSource_.sPath_.c_str());
		return false;
	}

	// Show info
	LOG->Info(
		LogLevel::LOW,
		"%s: %s, %dms, %dbits/s.",
		__FILE__,
		pFormatContext_->iformat->name,
		pFormatContext_->duration / (AV_TIME_BASE / 1000),
		pFormatContext_->bit_rate
	);

	// Read Packets from AVFormatContext to get stream information
	// avformat_find_streainfo populates pFormatContext_->streams (of size equals to pFormatContext->nb_streams)
	if (bDebug_)
		LOG->Info(LogLevel::LOW, "%s: Finding stream info from format", __FILE__);
	if (avformat_find_stream_info(pFormatContext_, nullptr) < 0) {
		LOG->Error("%s: Could not get the stream info.", __FILE__);
		return false;
	}

	for (unsigned int i = 0; i < pFormatContext_->nb_streams; ++i) {
		const auto pAVStream = pFormatContext_->streams[i];
		const auto pAVCodecParameters = pAVStream->codecpar;

		if (bDebug_) {
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
			if (VideoSource_.iVideoStreamIndex_ == -1)
				if (videoSource.iVideoStreamIndex_ == -1 || videoSource.iVideoStreamIndex_ == i) {
					LOG->Info(
						LogLevel::LOW,
						"%s: Using video stream #%d.",
						__FILE__,
						i
					);

					VideoSource_.iVideoStreamIndex_ = i;
					dFramerate_ = av_q2d(pAVStream->avg_frame_rate);
					dIntervalFrame_ = 1.0 / dFramerate_;
					pAVCodec_ = pAVCodec;
					pAVCodecParameters_ = pAVCodecParameters;
					iWidth_ = pAVCodecParameters->width;
					iHeight_ = pAVCodecParameters->height;
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

	pCodecContext_ = avcodec_alloc_context3(pAVCodec_);
	if (!pCodecContext_) {
		LOG->Error("%s: failed to allocated memory for AVCodecContext", __FILE__);
		return false;
	}
	pCodecContext_->thread_count = VideoSource_.uiDecodingThreadCount_;

	// Fill the codec context based on the values from the supplied codec parameters
	if (avcodec_parameters_to_context(pCodecContext_, pAVCodecParameters_) < 0) {
		LOG->Error("%s: failed to copy codec params to codec context", __FILE__);
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	if (avcodec_open2(pCodecContext_, pAVCodec_, nullptr) < 0) {
		LOG->Error("%s: failed to open codec through avcodec_open2", __FILE__);
		return false;
	}

	pFrame_ = av_frame_alloc();
	pGLFrame_ = av_frame_alloc();
	
	// Allocate te data buffer for the glFrame
	const auto iSize = av_image_get_buffer_size(
		AV_PIX_FMT_RGB24,
		pCodecContext_->width,
		pCodecContext_->height,
		1
	);

	const auto puiInternalBuffer = static_cast<const uint8_t*>(
		av_malloc(iSize * sizeof(uint8_t))
	);

	av_image_fill_arrays(
		pGLFrame_->data,
		pGLFrame_->linesize,
		puiInternalBuffer,
		AV_PIX_FMT_RGB24,
		pCodecContext_->width,
		pCodecContext_->height,
		1
	);

	if (!pFrame_ || !pGLFrame_) {
		LOG->Error("%s: failed to allocated memory for AVFrame", __FILE__);
		return false;
	}

	// Create the convert Context, used by the OpenGLFrame
	pConvertContext_ = sws_getContext(
		pCodecContext_->width,
		pCodecContext_->height,
		pCodecContext_->pix_fmt,	// Source
		pCodecContext_->width,
		pCodecContext_->height,
		AV_PIX_FMT_RGB24,			// Destiny (we change the format only)
		0,							// No interpolation
		nullptr,
		nullptr,
		nullptr
	);

	if (!pConvertContext_) {
		LOG->Error("%s: Could not create the convert context for OpenGL", __FILE__);
		return false;
	}

	pAVPacket_ = av_packet_alloc();
	if (!pAVPacket_) {
		LOG->Error("%s: Video: failed to allocated memory for AVPacket", __FILE__);
		return false;
	}

	// Allocate the OpenGL texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &uiTextureOGLName_);
	glBindTexture(GL_TEXTURE_2D, uiTextureOGLName_);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		pAVCodecParameters_->width,
		pAVCodecParameters_->height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	bLoaded_ = true;

	while (!bNewFrame_)
		decode();

	pWorkerThread_ = new std::thread([&] {
		while (!bStopWorkerThread_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // hack
			decode();
		}
		});

	return true;
}

void Video::decode()
{
	if (
		dTime_ < dNextFrameTime_ - renderInterval() ||
		dTime_ > dNextFrameTime_ + renderInterval() * 2
		) {
		LOG->Info(
			LogLevel::HIGH,
			"%s: Seeking %s[stream %d] @ %.4fs [desynced by %.4fs]...",
			__FILE__,
			VideoSource_.sPath_.c_str(),
			VideoSource_.iVideoStreamIndex_,
			dTime_,
			dTime_ - dNextFrameTime_
		);

		seekTime(dTime_);
		dNextFrameTime_ = 0.0f;
	}
	else if (dTime_ < dNextFrameTime_)
	{
		return;
	}

	if (bDebug_) {
		LOG->Info(
			LogLevel::LOW,
			"%s: Time: %.4fs, Next Frame time: %.4fs",
			__FILE__,
			dTime_,
			dNextFrameTime_
		);
	}

	// Retrieve new frame
	while (dNextFrameTime_ <= dTime_)
	{
		// fill the Packet with data from the Stream
		if (av_read_frame(pFormatContext_, pAVPacket_) >= 0) {
			// if it's the video stream
			if (pAVPacket_->stream_index == VideoSource_.iVideoStreamIndex_) {
				if (decodePacket() < 0)
					LOG->Error("%s: Packet cannot be decoded", __FILE__);
			}
		}
		else {
			// Loop: Start the video again
			seekTime(0);
			// av_seek_frame(pFormatContext, video_streaindex, 0, 0);
		}

		av_packet_unref(pAVPacket_);
	}
}

void Video::renderVideo(double dTime)
{
	dTime_ = dTime;

	if (!bLoaded_)
		return;
		
	if (pCodecContext_ && bNewFrame_) {
		glBindTextureUnit(0, uiTextureOGLName_);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			pCodecContext_->width,
			pCodecContext_->height,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			pGLFrame_->data[0]
		);
		bNewFrame_ = false;
	}
}

void Video::bind(GLuint uiTexUnit) const
{
	glBindTextureUnit(uiTexUnit, uiTextureOGLName_);
}

int64_t Video::seekTime(double dSeconds) const
{
	const auto iTimeMs = static_cast<int64_t>(dSeconds * 1000.);
	const auto iFrameNumber = av_rescale(
		iTimeMs,
		pFormatContext_->streams[VideoSource_.iVideoStreamIndex_]->time_base.den,
		pFormatContext_->streams[VideoSource_.iVideoStreamIndex_]->time_base.num
	) / 1000;

	if (av_seek_frame(pFormatContext_, VideoSource_.iVideoStreamIndex_, iFrameNumber, 0) < 0)
		LOG->Error("%s: Could not reach position: %.4fs, frame: %d", __FILE__, dSeconds, iFrameNumber);

	return iFrameNumber;
}

int32_t Video::decodePacket()
{
	// Supply raw packet data as input to a decoder
	auto iResponse = avcodec_send_packet(pCodecContext_, pAVPacket_);

	if (iResponse < 0) {
		LOG->Error("%s: decodePacket Error while sending a packet to the decoder", __FILE__); // : %s", av_err2str(response));
		return iResponse;
	}

	while (iResponse >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		iResponse = avcodec_receive_frame(pCodecContext_, pFrame_);
		if (iResponse == AVERROR(EAGAIN) || iResponse == AVERROR_EOF)
			break;
		else if (iResponse < 0) {
			LOG->Error("%s: Error while receiving a frame from the decoder", __FILE__); // : %s", av_err2str(response));
			return iResponse;
		}

		if (iResponse >= 0) {
			if (bDebug_) {
				LOG->Info(
					LogLevel::LOW,
					"%s: Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
					__FILE__,
					pCodecContext_->frame_number,
					av_get_picture_type_char(pFrame_->pict_type),
					pFrame_->pkt_size,
					pFrame_->pts,
					pFrame_->key_frame,
					pFrame_->coded_picture_number
				);
			}

			// Scale the image (pFrame) to the OpenGL image (glFrame), using the Convertex cotext
			sws_scale(
				pConvertContext_,
				pFrame_->data,
				pFrame_->linesize,
				0,
				pCodecContext_->height,
				pGLFrame_->data,
				pGLFrame_->linesize
			);

			bNewFrame_ = true;
			//dNextFrameTime_ = static_cast<double>(pCodecContext_->frame_number) * renderInterval();
			dNextFrameTime_ = dTime_ + renderInterval();
		}
	}

	return 0;
}

std::string const& Video::getFileName() const
{
	return VideoSource_.sPath_;
}

GLuint Video::getTexID() const
{
	return uiTextureOGLName_;
}

int32_t Video::getWidth() const
{
	return iWidth_;
}

int32_t Video::getHeight() const
{
	return iHeight_;
}
