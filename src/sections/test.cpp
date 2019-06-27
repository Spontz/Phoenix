#include "main.h"
#include "core/video.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#define INBUF_SIZE 4096

typedef struct {
	Texture tex;

	const AVCodec *codec;
	AVCodecParserContext *parser;
	AVCodecContext *c = NULL;
	AVFrame *frame;
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	uint8_t *data;
	size_t   data_size;
	int ret;
	AVPacket *pkt;

} test_section;

static test_section *local;

sTest::sTest() {
	type = SectionType::Test;
}





// The flush packet is a non-NULL packet with size 0 and data NULL
int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame, AVPacket *pkt)
{
	int ret;

	*got_frame = 0;

	if (pkt) {
		ret = avcodec_send_packet(avctx, pkt);
		// In particular, we don't expect AVERROR(EAGAIN), because we read all
		// decoded frames with avcodec_receive_frame() until done.
		if (ret < 0)
			return ret == AVERROR_EOF ? 0 : ret;
	}

	ret = avcodec_receive_frame(avctx, frame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		return ret;
	if (ret >= 0)
		*got_frame = 1;

	return 0;
}



bool sTest::load() {
	local = (test_section*)malloc(sizeof(test_section));

	this->vars = (void *)local;
	
	//if (local->shader < 0)
	//	return false;
	string file = DEMO->dataFolder + this->strings[0];
	FILE *f;

	local->pkt = av_packet_alloc();
	if (!local->pkt) {
		LOG->Error("Video error: Could not allocate packet");
		return false;
	}



	return true;
}

void sTest::init() {
}


void sTest::exec() {
	local = (test_section *)this->vars;
	EvalBlendingStart();
	glDisable(GL_DEPTH_TEST);
	{
		// Load the background texture
		//Texture *my_tex = DEMO->textureManager.texture[local->texture];
		
		// Read frame
		//int readed = readFrame(&(local->data));

		// Texture and View aspect ratio, stored for Keeping image proportions
		float tex_aspect = (float)local->data.codec_ctx->width / (float)local->data.codec_ctx->height;
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
		Shader *my_shad = DEMO->shaderManager.shader[RES->shdr_QuadTexModel];
		my_shad->use();
		my_shad->setValue("model", model);
		my_shad->setValue("screenTexture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, local->data.gl_frame_texID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	glEnable(GL_DEPTH_TEST);
	EvalBlendingEnd();
	
}

void sTest::end() {
}
