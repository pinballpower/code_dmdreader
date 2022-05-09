
/*
 * Copyright (c) 2021 Pinball Power
 * Copyright (c) 2017 Jun Zhao
 * Copyright (c) 2017 Kaixuan Liu
 *
 * HW Acceleration API (video decoding) decode sample
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
}

#include <boost/log/trivial.hpp>

#include "drmhelper.h"
#include "videoplayer.h"


using namespace std;

static enum AVPixelFormat hw_pix_fmt;

static int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
	int err = 0;

	ctx->hw_frames_ctx = NULL;
	// ctx->hw_device_ctx gets freed when we call avcodec_free_context
	if ((err = av_hwdevice_ctx_create(&ctx->hw_device_ctx, type,
		NULL, NULL, 0)) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to create specified HW device.";
		return err;
	}

	return err;
}

static enum AVPixelFormat get_hw_format(AVCodecContext* ctx,
	const enum AVPixelFormat* pix_fmts)
{
	const enum AVPixelFormat* p;

	for (p = pix_fmts; *p != -1; p++) {
		if (*p == hw_pix_fmt)
			return *p;
	}

	BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to get HW surface format.";
	return AV_PIX_FMT_NONE;
}

static int decode_write(AVCodecContext* const avctx,
	DRMPrimeOut* const dpo,
	AVPacket* packet)
{
	AVFrame* frame = NULL, * sw_frame = NULL;
	uint8_t* buffer = NULL;
	int size;
	int ret = 0;
	unsigned int i;

	ret = avcodec_send_packet(avctx, packet);
	if (ret < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] error during decoding";
		return ret;
	}

	for (;;) {
		if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] can not alloc frame";
			ret = AVERROR(ENOMEM);
			goto fail;
		}

		ret = avcodec_receive_frame(avctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_frame_free(&frame);
			av_frame_free(&sw_frame);
			return 0;
		}
		else if (ret < 0) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] error while decoding";
			goto fail;
		}

		dpo->displayFrame(frame);

	fail:
		av_frame_free(&frame);
		av_frame_free(&sw_frame);
		av_freep(&buffer);
		if (ret < 0)
			return ret;
	}
	return 0;
}


bool VideoPlayer::playLoop(string filename, int loopCount)
{
	AVFormatContext* input_ctx = NULL;
	int video_stream, ret;
	AVStream* video = NULL;
	AVCodecContext* decoder_ctx = NULL;
	AVCodec* decoder = NULL;
	AVPacket packet;
	enum AVHWDeviceType type;
	const char* hwdev = "drm";
	int i;

	openScreen();

	type = av_hwdevice_find_type_by_name(hwdev);
	if (type == AV_HWDEVICE_TYPE_NONE) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] device type " << hwdev << " is not supported.";
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] available device types:";
		while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
			BOOST_LOG_TRIVIAL(error) << "              " << av_hwdevice_get_type_name(type);
		return false;
	}

loopy:

	/* open the input file */
	if (avformat_open_input(&input_ctx, filename.c_str(), NULL, NULL) != 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] cannot open input file " << filename;
		return false;
	}

	if (avformat_find_stream_info(input_ctx, NULL) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] cannot find input stream information.";
		return false;
	}

	/* find the video stream information */
	ret = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
	if (ret < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] cannot find a video stream in the input file";
		return false;
	}
	video_stream = ret;

	if (decoder->id == AV_CODEC_ID_H264) {
		if ((decoder = avcodec_find_decoder_by_name("h264_v4l2m2m")) == NULL) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] cannot find the h264 v4l2m2m decoder";
			return false;
		}
		hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
	}
	else {
		for (i = 0;; i++) {
			const AVCodecHWConfig* config = avcodec_get_hw_config(decoder, i);
			if (!config) {
				BOOST_LOG_TRIVIAL(error) << "[videoplayer] decoder " << decoder->name << " does not support device type " << av_hwdevice_get_type_name(type);
				return false;
			}
			if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
				config->device_type == type) {
				hw_pix_fmt = config->pix_fmt;
				break;
			}
		}
	}

	if (!(decoder_ctx = avcodec_alloc_context3(decoder))) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't allocate AV codec";
		return false;
	}

	video = input_ctx->streams[video_stream];
	if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't get context";
		return false;
	}

	decoder_ctx->get_format = get_hw_format;

	if (hw_decoder_init(decoder_ctx, type) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't initialize HW decoder";
		return false;
	}

	decoder_ctx->thread_count = 3;

	if ((ret = avcodec_open2(decoder_ctx, decoder, NULL)) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to open codec for stream #" << video_stream;
		return -1;
	}

	/* actual decoding */
	playing = true;
	while ((ret >= 0) and (playing)) {
		if ((ret = av_read_frame(input_ctx, &packet)) < 0)
			break;

		if (video_stream == packet.stream_index)
			ret = decode_write(decoder_ctx, dpo, &packet);

		av_packet_unref(&packet);
	}

	/* flush the decoder */
	packet.data = NULL;
	packet.size = 0;
	ret = decode_write(decoder_ctx, dpo, &packet);
	av_packet_unref(&packet);

	avcodec_free_context(&decoder_ctx);
	avformat_close_input(&input_ctx);

	playing = false;

	if (loopCount != 0) {
		loopCount--;
		goto loopy;
	}

	closeScreen();

	return true;
}

VideoPlayer::VideoPlayer()
{
	playing = false;
}

bool VideoPlayer::openScreen()
{
	if (!(screenOpened)) {
		compose_t compose{ x,y,width,height };
		dpo = new DRMPrimeOut(compose);
		if (dpo == NULL) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to open drmprime output";
			return false;
		}
		else {
			BOOST_LOG_TRIVIAL(debug) << "[videoplayer] opened drmprime output";
		}
	}
	else {
		BOOST_LOG_TRIVIAL(trace) << "[videoplayer] screen already initialized";
	}
	screenOpened = true;
	return true;
}

void VideoPlayer::closeScreen() {
	screenOpened = false;
	delete dpo;
}

void VideoPlayer::play(string filename, int loopCount)
{
	playLoop(filename, loopCount);
}

void VideoPlayer::playBackground(string filename, int loopCount)
{
	playerThread = thread(&VideoPlayer::playLoop, this, filename, loopCount);
}

bool VideoPlayer::isPlaying()
{
	return playing;
}

void VideoPlayer::stop()
{
	playing = false;
}

void VideoPlayer::setScaling(int x, int y, int width, int height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}
