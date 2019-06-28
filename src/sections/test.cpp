#include "main.h"
#include "core/video.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

typedef struct {
	GLuint texID;

	AVFormatContext *pFormatContext;		// AVFormatContext holds the header information from the format (Container)
	AVCodec *pCodec;						// The component that knows how to enCOde and DECode the stream is the codec
	AVCodecParameters *pCodecParameters;	// This component describes the properties of a codec used by the stream i
	int video_stream_index;
	AVCodecContext *pCodecContext;			// Codec context
	AVFrame *pFrame;						// AV Frame
	AVFrame *glFrame;						// OpenGL Frame
	SwsContext *conv_ctx;					// Convert Context (for OpenGL)
	AVPacket *pPacket;						// Packet
	float framerate;
} test_section;

static test_section *local;

////////////////////////////////////////////

static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, SwsContext *conv_ctx, AVFrame *glFrame, GLuint gl_textureID)
{
	// Supply raw packet data as input to a decoder
	// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
	int response = avcodec_send_packet(pCodecContext, pPacket);

	if (response < 0) {
		LOG->Error("Error while sending a packet to the decoder");// : %s", av_err2str(response));
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
			LOG->Info(LOG_LOW,
				"Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
				pCodecContext->frame_number,
				av_get_picture_type_char(pFrame->pict_type),
				pFrame->pkt_size,
				pFrame->pts,
				pFrame->key_frame,
				pFrame->coded_picture_number
			);
			*/
			// Scale the image (pFrame) to the OpenGL image (glFrame), using the Convertex cotext
			sws_scale(conv_ctx, pFrame->data, pFrame->linesize, 0, pCodecContext->height, glFrame->data, glFrame->linesize);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gl_textureID);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecContext->width, pCodecContext->height, GL_RGB, GL_UNSIGNED_BYTE, glFrame->data[0]);

			//av_frame_unref(pFrame);
			//av_frame_unref(glFrame);
		}
	}
	return 0;
}

bool seekMs(int tsms, AVFormatContext *pFormatContext, int streamID)
{
	//printf("**** SEEK TO ms %d. LLT: %d. LT: %d. LLF: %d. LF: %d. LastFrameOk: %d\n",tsms,LastLastFrameTime,LastFrameTime,LastLastFrameNumber,LastFrameNumber,(int)LastFrameOk);

	// Convert time into frame number
	int64_t DesiredFrameNumber;
	DesiredFrameNumber = av_rescale(tsms, pFormatContext->streams[streamID]->time_base.den, pFormatContext->streams[streamID]->time_base.num);
	DesiredFrameNumber /= 1000;
	
	float second = (float)tsms / 1000.0f;

	if (av_seek_frame(pFormatContext, streamID, DesiredFrameNumber, 0) < 0) {
		LOG->Error("Video: Could not reach position: %f, frame: %d", second, DesiredFrameNumber);
		return false;
	}
	
	return true;
	
}

/////////////////////////////////////////////////




sTest::sTest() {
	type = SectionType::Test;
}






bool sTest::load() {
	local = (test_section*)malloc(sizeof(test_section));

	this->vars = (void *)local;
	
	//if (local->shader < 0)
	//	return false;
	string file = DEMO->dataFolder + this->strings[0];

	local->pCodec = NULL;
	local->pCodecParameters = NULL;
	local->video_stream_index = -1;
	local->conv_ctx = NULL;


	// Allocate memory for the Context: http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
	local->pFormatContext = avformat_alloc_context();

	if (!local->pFormatContext) {
		LOG->Error("Video: ERROR could not allocate memory for Format Context");
		return false;
	}

	LOG->Info(LOG_LOW, "Video: Opening the input file (%s) and loading format (container) header", file.c_str());
	// Open the file and read its header. The codecs are not opened: http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
	if (avformat_open_input(&local->pFormatContext, file.c_str(), NULL, NULL) != 0) {
		LOG->Error("Video: ERROR could not open the file %s", file.c_str());
		return false;
	}

	// Show info file
	LOG->Info(LOG_LOW, "Video: Format %s, duration %lld us, bit_rate %lld", local->pFormatContext->iformat->name, local->pFormatContext->duration, local->pFormatContext->bit_rate);

	LOG->Info(LOG_LOW, "Video: Finding stream info from format");
	// Read Packets from the Format to get stream information
	// this function populates pFormatContext->streams (of size equals to pFormatContext->nb_streams)
	// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
	if (avformat_find_stream_info(local->pFormatContext, NULL) < 0) {
		LOG->Error("Video: ERROR could not get the stream info");
		return false;
	}

	// loop though all the streams and print its main information
	for (unsigned int i = 0; i < local->pFormatContext->nb_streams; i++) {
		AVCodecParameters *pLocalCodecParameters = NULL;
		pLocalCodecParameters = local->pFormatContext->streams[i]->codecpar;
		LOG->Info(LOG_LOW, "Video: AVStream->time_base before open coded %d/%d", local->pFormatContext->streams[i]->time_base.num, local->pFormatContext->streams[i]->time_base.den);
		LOG->Info(LOG_LOW, "Video: AVStream->r_frame_rate before open coded %d/%d", local->pFormatContext->streams[i]->r_frame_rate.num, local->pFormatContext->streams[i]->r_frame_rate.den);
		LOG->Info(LOG_LOW, "Video: AVStream->start_time %" PRId64, local->pFormatContext->streams[i]->start_time);
		LOG->Info(LOG_LOW, "Video: AVStream->duration %" PRId64, local->pFormatContext->streams[i]->duration);

		LOG->Info(LOG_LOW,"Video: finding the proper decoder (CODEC)");

		AVCodec *pLocalCodec = NULL;

		// finds the registered decoder for a codec ID
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
		pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

		if (pLocalCodec == NULL) {
			LOG->Error("Video: ERROR unsupported codec!");
			return false;
		}

		// when the stream is a video we store its index, codec parameters and codec
		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (local->video_stream_index == -1) {
				local->video_stream_index = i;
				local->framerate = (float)av_q2d(local->pFormatContext->streams[i]->avg_frame_rate);
				local->pCodec = pLocalCodec;
				local->pCodecParameters = pLocalCodecParameters;
			}

			LOG->Info(LOG_LOW, "Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
		}
		else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
			LOG->Info(LOG_LOW, "Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
		}

		// print its name, id and bitrate
		LOG->Info(LOG_LOW, "Video: Codec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, local->pCodecParameters->bit_rate);
	}
	// https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
	local->pCodecContext = avcodec_alloc_context3(local->pCodec);
	if (!local->pCodecContext) {
		LOG->Error("Video: failed to allocated memory for AVCodecContext");
		return false;
	}

	// Fill the codec context based on the values from the supplied codec parameters
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
	if (avcodec_parameters_to_context(local->pCodecContext, local->pCodecParameters) < 0) {
		LOG->Error("Video: failed to copy codec params to codec context");
		return false;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
	if (avcodec_open2(local->pCodecContext, local->pCodec, NULL) < 0) {
		LOG->Error("Video: failed to open codec through avcodec_open2");
		return false;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVFrame.html
	local->pFrame = av_frame_alloc();
	local->glFrame = av_frame_alloc();
	// Allocate te data buffer for the glFrame
	int size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, local->pCodecContext->width, local->pCodecContext->height, 1);
	uint8_t *internal_buffer = (uint8_t *)av_malloc(size * sizeof(uint8_t));
	av_image_fill_arrays((uint8_t**)((AVPicture *)local->glFrame->data), (int*)((AVPicture *)local->glFrame->linesize), internal_buffer, AV_PIX_FMT_RGB24, local->pCodecContext->width, local->pCodecContext->height, 1);
	if ((!local->pFrame) || (!local->glFrame)) {
		LOG->Error("Video: failed to allocated memory for AVFrame");
		return false;
	}


	// Create the convert Context, used by the OpenGLFrame
	local->conv_ctx = sws_getContext(	local->pCodecContext->width, local->pCodecContext->height, local->pCodecContext->pix_fmt,	// Source
										local->pCodecContext->width, local->pCodecContext->height, AV_PIX_FMT_RGB24,				// Destiny (we change the format only)
										SWS_BICUBIC, NULL, NULL, NULL);
	if (!local->conv_ctx) {
		LOG->Error("Could not create the convert context for OpenGL");
		return false;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVPacket.html
	local->pPacket = av_packet_alloc();
	if (!local->pPacket) {
		LOG->Error("Video: failed to allocated memory for AVPacket");
		return false;
	}


	// Allocate the OpenGL texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &local->texID);
	glBindTexture(GL_TEXTURE_2D, local->texID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, local->pCodecParameters->width, local->pCodecParameters->height,	0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	

/*	int response = 0;
	int how_many_packets_to_process = 8;

	// fill the Packet with data from the Stream
	// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
	while (av_read_frame(local->pFormatContext, local->pPacket) >= 0) {
		// if it's the video stream
		if (local->pPacket->stream_index == local->video_stream_index) {
			LOG->Info(LOG_LOW, "Video: AVPacket->pts %" PRId64, local->pPacket->pts);
			response = decode_packet(local->pPacket, local->pCodecContext, local->pFrame);
			if (response < 0)
				break;
			// stop it, otherwise we'll be saving hundreds of frames
			if (--how_many_packets_to_process <= 0) break;
		}
		// https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
		av_packet_unref(local->pPacket);
	}
	
	LOG->Info(LOG_LOW, "Video: releasing all the resources");

	*/
	/* //TODO: Add this in the Video Class!!!
	avformat_close_input(&local->pFormatContext);
	avformat_free_context(local->pFormatContext);
	av_packet_free(&local->pPacket);
	av_frame_free(&local->pFrame);
	av_frame_free(&local->glFrame);
	avcodec_free_context(&local->pCodecContext);
	*/
	return true;
}

void sTest::init() {
}

int played_frames = 0;
float last_RenderedTime = 0.0;

void sTest::exec() {
	local = (test_section *)this->vars;


	// Time we should draw a new frame
	float interval_frame = 1/local->framerate;

	float time = last_RenderedTime;
	// If a new frame needs to be rendered... we capture de frame
	if (this->runTime >= (last_RenderedTime + interval_frame)) {
		LOG->Info(LOG_LOW, "Render time: %.4f, Last Render: %.4f, next Render will be: %.4f", this->runTime, last_RenderedTime, this->runTime+interval_frame);
		// Check if frameskip is detected... in that case, we seek for the right frame
		if (this->runTime >= (last_RenderedTime + (interval_frame * 2.0f))) {
			seekMs((int)(this->runTime*1000.0f), local->pFormatContext, local->video_stream_index);
		}
		last_RenderedTime = this->runTime;
		int response = 0;
		int how_many_packets_to_process = 8;

		// fill the Packet with data from the Stream
		// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
		if (av_read_frame(local->pFormatContext, local->pPacket) >= 0) {
			// if it's the video stream
			if (local->pPacket->stream_index == local->video_stream_index) {
				//LOG->Info(LOG_LOW, "Video: AVPacket->pts %" PRId64, local->pPacket->pts);
				response = decode_packet(local->pPacket, local->pCodecContext, local->pFrame, local->conv_ctx, local->glFrame, local->texID);
				if (response < 0)
					LOG->Error("Video: Packet cannot be decoded");
			}
		}
		else {
			// Loop: Start the video again
			av_seek_frame(local->pFormatContext, local->video_stream_index, 0, 0);
		}
		// https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
		av_packet_unref(local->pPacket);
	}
	if (this->runTime < last_RenderedTime) {
		last_RenderedTime = this->runTime;
		seekMs((int)(this->runTime*1000.0f), local->pFormatContext, local->video_stream_index);
	}
		

	
	
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Load the background texture
		//Texture *my_tex = DEMO->textureManager.texture[local->texture];
		
		// Read frame
		//int readed = readFrame(&(local->data));

		// Texture and View aspect ratio, stored for Keeping image proportions
		float tex_aspect = (float)local->pCodecParameters->width / (float)local->pCodecParameters->height;
		float view_aspect = (float)GLDRV->width / (float)GLDRV->height;

		// Put orthogonal mode
		glm::mat4 model = glm::mat4(1.0f);

		// Change the model matrix, in order to scale the image and keep proportions of the image
		float new_tex_width_scaled = 1;
		float new_tex_height_scaled = 1;
		if (1) {//local->mode == background_drawing_mode__fit_to_content) {
			if (tex_aspect > view_aspect) {
				new_tex_height_scaled = view_aspect / tex_aspect;
			}
			else {
				new_tex_width_scaled = tex_aspect / view_aspect;
			}
		}
		model = glm::scale(model, glm::vec3(new_tex_width_scaled, new_tex_height_scaled, 0.0f));

		//RES->Draw_Obj_QuadTex(local->texture, &model);
		glBindVertexArray(RES->obj_quadFullscreen);
		Shader *my_shad = DEMO->shaderManager.shader[RES->shdr_QuadTexVFlipModel];
		my_shad->use();
		my_shad->setValue("model", model);
		my_shad->setValue("screenTexture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, local->texID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	glEnable(GL_DEPTH_TEST);

	EvalBlendingEnd();
	
}

void sTest::end() {
}
