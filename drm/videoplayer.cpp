
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
#include <chrono>
#include <thread>

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

#include "videoplayer.hpp"

using namespace std;

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


bool VideoPlayer::playLoop(VideoFile *videoFile, bool loop)
{
	int ret;
	
	
	AVPacket packet;
	const char* hwdev = "drm";
	int i;

	openScreen();

	terminate = false;

	while (!terminate) {



		if (hw_decoder_init(videoFile->decoderContext, AV_HWDEVICE_TYPE_DRM) < 0) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't initialize HW decoder";
			return false;
		}

		BOOST_LOG_TRIVIAL(error) << "[t4]" << duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		videoFile->decoderContext->thread_count = 3;

		if ((ret = avcodec_open2(videoFile->decoderContext, videoFile->decoder, NULL)) < 0) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to open codec for stream #" << videoFile->videoStream;
			return -1;
		}

		BOOST_LOG_TRIVIAL(error) << "[t5]" << duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();




		/* actual decoding */
		playing = true;
		while ((ret >= 0) and (playing)) {
			if ((ret = av_read_frame(videoFile->inputContext, &packet)) < 0)
				break;

			// very simplistic pause implementation
			while (paused) {
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
			}

			if (videoFile->videoStream == packet.stream_index)
				ret = decode_write(videoFile->decoderContext, dpo, &packet);

			av_packet_unref(&packet);
		}

		/* flush the decoder */
		packet.data = NULL;
		packet.size = 0;
		ret = decode_write(videoFile->decoderContext, dpo, &packet);
		av_packet_unref(&packet);

		//avcodec_free_context(&decoder_ctx);
		// avformat_close_input(&input_ctx);

		if (!loop) {
			terminate = true;
		}
	}

	closeScreen();
	playing = false;


	return true;
}

VideoPlayer::VideoPlayer(int screenNumber, int planeNumber, CompositionGeometry compositionGeometry)
{
	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;
	this->compositionGeometry = compositionGeometry;
	playing = false;
	paused = false;
}

VideoPlayer::~VideoPlayer()
{
	stop();
	closeScreen();
}

bool VideoPlayer::openScreen()
{
	if (!(screenOpened)) {
		dpo = new DRMPrimeOut(compositionGeometry, screenNumber, planeNumber);
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


void VideoPlayer::startPlayback(VideoFile &videoFile,  bool loop)
{
	stop();
	playerThread = thread(&VideoPlayer::playLoop, this, &videoFile, loop);
}

bool VideoPlayer::isPlaying()
{
	return playerThread.joinable();
}

void VideoPlayer::stop()
{
	terminate = true;

	// finish player thread
	if (playerThread.joinable()) {
		playerThread.join();
	}
}

void VideoPlayer::pause(bool paused)
{
	this->paused = paused;
}

void VideoPlayer::setComposition(CompositionGeometry compositionGeometry)
{
	this->compositionGeometry = compositionGeometry;
}