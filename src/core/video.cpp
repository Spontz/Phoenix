// video.cpp
// Spontz Demogroup

#include <main.h>
#include <io.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include "video.h"

Video::Video()
{
	fileName = "Video not loaded";
	pCodec = NULL;
	pCodecParameters = NULL;
	video_stream_index = -1;
	conv_ctx = NULL;
	framerate = 0;
	width = height = 0;
	intervalFrame = 0;
	lastRenderTime = 0;
	loaded = false;
	texID = 0;
}

Video::~Video()
{
	if (texID != 0) {
		glDeleteTextures(1, &texID);
		texID = 0;
	}
	avformat_close_input(&pFormatContext);
	avformat_free_context(pFormatContext);
	
	av_packet_unref(pPacket);
	av_packet_free(&pPacket);

	av_frame_unref(pFrame);
	av_frame_unref(glFrame);
	av_frame_free(&pFrame);
	av_frame_free(&glFrame);

	avcodec_free_context(&pCodecContext);
}

bool Video::load(const std::string & filename)
{
	this->fileName = filename;
	// Allocate memory for the Context: http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
	pFormatContext = avformat_alloc_context();

	if (!pFormatContext) {
		LOG->Error("Video: could not allocate memory for Format Context");
		return false;
	}

	//LOG->Info(LogLevel::LOW, "Video: Opening the input file (%s) and loading format (container) header", fileName.c_str());
	// Open the file and read its header. The codecs are not opened: http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
	if (avformat_open_input(&pFormatContext, fileName.c_str(), NULL, NULL) != 0) {
		LOG->Error("Video: could not open the file %s", fileName.c_str());
		return false;
	}

	// Show info file
	LOG->Info(LogLevel::LOW, "Video: Format %s, duration %lld us, bit_rate %lld", pFormatContext->iformat->name, pFormatContext->duration, pFormatContext->bit_rate);

	//LOG->Info(LogLevel::LOW, "Video: Finding stream info from format");
	// Read Packets from the Format to get stream information
	// this function populates pFormatContext->streams (of size equals to pFormatContext->nb_streams)
	// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
	if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
		LOG->Error("Video: could not get the stream info");
		return false;
	}

	// loop though all the streams and print its main information
	for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
		AVCodecParameters *pLocalCodecParameters = NULL;
		pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
		//LOG->Info(LogLevel::LOW, "Video: AVStream->time_base before open coded %d/%d", pFormatContext->streams[i]->time_base.num, pFormatContext->streams[i]->time_base.den);
		//LOG->Info(LogLevel::LOW, "Video: AVStream->r_frame_rate before open coded %d/%d", pFormatContext->streams[i]->r_frame_rate.num, pFormatContext->streams[i]->r_frame_rate.den);
		//LOG->Info(LogLevel::LOW, "Video: AVStream->start_time %" PRId64, pFormatContext->streams[i]->start_time);
		//LOG->Info(LogLevel::LOW, "Video: AVStream->duration %" PRId64, pFormatContext->streams[i]->duration);
		//LOG->Info(LogLevel::LOW, "Video: finding the proper decoder (CODEC)");

		AVCodec *pLocalCodec = NULL;

		// finds the registered decoder for a codec ID
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
		pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

		if (pLocalCodec == NULL) {
			LOG->Error("Video: unsupported codec!");
			return false;
		}

		// when the stream is a video we store its index, codec parameters and codec
		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (video_stream_index == -1) {
				video_stream_index = i;
				framerate = (float)av_q2d(pFormatContext->streams[i]->avg_frame_rate);
				intervalFrame = 1.0f / framerate;
				pCodec = pLocalCodec;
				pCodecParameters = pLocalCodecParameters;
				width = pLocalCodecParameters->width;
				height = pLocalCodecParameters->height;
			}
			LOG->Info(LogLevel::LOW, "Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
		}
		// There is no need to play audio
		//else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
		//	LOG->Info(LogLevel::LOW, "Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
		//}

		// print its name, id and bitrate
		LOG->Info(LogLevel::LOW, "Video: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pCodecParameters->bit_rate);
	}
	// https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
	pCodecContext = avcodec_alloc_context3(pCodec);
	if (!pCodecContext) {
		LOG->Error("Video: failed to allocated memory for AVCodecContext");
		return false;
	}

	// Fill the codec context based on the values from the supplied codec parameters
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
	if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
		LOG->Error("Video: failed to copy codec params to codec context");
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
	if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
		LOG->Error("Video: failed to open codec through avcodec_open2");
		return false;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVFrame.html
	pFrame = av_frame_alloc();
	glFrame = av_frame_alloc();
	// Allocate te data buffer for the glFrame
	int size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);
	uint8_t *internal_buffer = (uint8_t *)av_malloc(size * sizeof(uint8_t));
	av_image_fill_arrays((uint8_t**)((AVPicture *)glFrame->data), (int*)((AVPicture *)glFrame->linesize), internal_buffer, AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);
	if ((!pFrame) || (!glFrame)) {
		LOG->Error("Video: failed to allocated memory for AVFrame");
		return false;
	}


	// Create the convert Context, used by the OpenGLFrame
	conv_ctx = sws_getContext(pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt,	// Source
		pCodecContext->width, pCodecContext->height, AV_PIX_FMT_RGB24,				// Destiny (we change the format only)
		SWS_BICUBIC, NULL, NULL, NULL);
	if (!conv_ctx) {
		LOG->Error("Could not create the convert context for OpenGL");
		return false;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVPacket.html
	pPacket = av_packet_alloc();
	if (!pPacket) {
		LOG->Error("Video: failed to allocated memory for AVPacket");
		return false;
	}


	// Allocate the OpenGL texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pCodecParameters->width, pCodecParameters->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	loaded = true;
	return true;
}

void Video::renderVideo(float time)
{
	if (loaded) {
		// If we are rewinding...
		if (time < lastRenderTime) {
			lastRenderTime = time;
			seekTime(time);
		}

		// If a new frame needs to be rendered... we capture de frame
		if (time >= (lastRenderTime + intervalFrame)) {
			//LOG->Info(LogLevel::LOW, "Render time: %.4f, Last Render: %.4f, next Render will be: %.4f", time, lastRenderTime, time + intervalFrame);
			// Check if frameskip is detected... in that case, we seek for the right frame
			if (time >= (lastRenderTime + (intervalFrame * 2.0f))) {
				LOG->Info(LogLevel::LOW, "Seek needed!");
				seekTime(time);
			}
			lastRenderTime = time;
			int response = 0;

			// fill the Packet with data from the Stream
			// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
			if (av_read_frame(pFormatContext, pPacket) >= 0) {
				// if it's the video stream
				if (pPacket->stream_index == video_stream_index) {
					response = decodePacket();
					if (response < 0)
						LOG->Error("Video: Packet cannot be decoded");
				}
			}
			else {
				// Loop: Start the video again
				seekTime(0); //av_seek_frame(pFormatContext, video_stream_index, 0, 0);
			}
			// https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
			av_packet_unref(pPacket);
		}

	}
	
}

void Video::bind(int texUnit) const
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texID);
}

void Video::seekTime(float time)
{
	int timeMs = (int)(time * 1000.0f);
	int64_t DesiredFrameNumber;
	DesiredFrameNumber = av_rescale(timeMs, pFormatContext->streams[video_stream_index]->time_base.den, pFormatContext->streams[video_stream_index]->time_base.num);
	DesiredFrameNumber /= 1000;

	if (av_seek_frame(pFormatContext, video_stream_index, DesiredFrameNumber, 0) < 0) {
		LOG->Error("Video: Could not reach position: %f, frame: %d", time, DesiredFrameNumber);
	}
}

int Video::decodePacket()
{
	// Supply raw packet data as input to a decoder
	// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
	int response = avcodec_send_packet(pCodecContext, pPacket);

	if (response < 0) {
		LOG->Error("Video::decodePacket Error while sending a packet to the decoder");// : %s", av_err2str(response));
		return response;
	}

	while (response >= 0)
	{
		// Return decoded output data (into a frame) from a decoder
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
		response = avcodec_receive_frame(pCodecContext, pFrame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		}
		else if (response < 0) {
			LOG->Error("Error while receiving a frame from the decoder");// : %s", av_err2str(response));
			return response;
		}

		if (response >= 0) {
			/*
			LOG->Info(LogLevel::LOW,
				"Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
				pCodecContext->frame_number, av_get_picture_type_char(pFrame->pict_type), pFrame->pkt_size,
				pFrame->pts, pFrame->key_frame, pFrame->coded_picture_number );
			*/
			// Scale the image (pFrame) to the OpenGL image (glFrame), using the Convertex cotext
			sws_scale(conv_ctx, pFrame->data, pFrame->linesize, 0, pCodecContext->height, glFrame->data, glFrame->linesize);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texID);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecContext->width, pCodecContext->height, GL_RGB, GL_UNSIGNED_BYTE, glFrame->data[0]);

			//av_frame_unref(pFrame);
			//av_frame_unref(glFrame);
		}
	}
	return 0;
}
