
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

static int decode_write(AVCodecContext* const avctx,
	DRMPrimeOut* const dpo,
	AVPacket* packet)
{
	AVFrame* frame = NULL, * sw_frame = NULL;
	uint8_t* buffer = NULL;
	int size;
	int ret = 0;
	unsigned int i;
	int countFrames = 0;

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
			break;
		}
		else if (ret < 0) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] error while decoding";
			goto fail;
		}

		dpo->displayFrame(frame);
		countFrames++;

	fail:
		av_frame_free(&frame);
		av_frame_free(&sw_frame);
		av_freep(&buffer);
		if (ret < 0)
			return ret;
	}

	BOOST_LOG_TRIVIAL(trace) << "[videoplayer] frames decoded from packet: " << countFrames;
	return 0;
}


void VideoPlayer::playLoop(bool loop)
{
	AVPacket packet;

	terminate = false;

	/* actual decoding */
	playing = true;
	while (! terminate) {

		// eof check and loop
		if (!currentVideo->nextFrame(&packet)) {
			// got no package, probably EOF
			if (loop) {
				// try to read from the beginning
				currentVideo->seek(0);
				if (!currentVideo->nextFrame(&packet)) {
					break;
				}
			}
			else {
				break;
			}
		}

		// very simplistic pause implementation
		while (paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}

		if (currentVideo->videoStream == packet.stream_index) {
			// Theoretically a packet could contain more than one frame. However, with MP4 files it seems, a packet conatins
			// only one frame
			if (decode_write(currentVideo->decoderContext, dpo, &packet)) {
				BOOST_LOG_TRIVIAL(error) << "[videoplayer] error while decoding video stream, aborting";
				break;
			}
		}

		av_packet_unref(&packet);
	}

	/* flush the decoder */
	packet.data = NULL;
	packet.size = 0;
	decode_write(currentVideo->decoderContext, dpo, &packet);
	av_packet_unref(&packet);

	currentVideo->close();

	playing = false;
}

VideoPlayer::VideoPlayer(int screenNumber, int planeNumber, CompositionGeometry compositionGeometry)
{
	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;
	this->compositionGeometry = compositionGeometry;

	openScreen();

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


void VideoPlayer::startPlayback(unique_ptr<VideoFile> videoFile,  bool loop)
{
	if (! videoFile) {
		return;
	}

	videoFile->connectToDecoder();
	if (videoFile->getPlaybackState() != VideoPlaybackState::DECODER_CONNECTED) {
		BOOST_LOG_TRIVIAL(trace) << "[videoplayer] couldn't connect to video decoder";
		return;
	}
	stop();
	currentVideo = std::move(videoFile);
	playerThread = thread(&VideoPlayer::playLoop, this, loop);
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