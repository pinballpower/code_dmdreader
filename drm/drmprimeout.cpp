/*
 * Copyright (c) 2022 Pinball Power
 * Copyright (c) 2020 John Cox for Raspberry Pi Trading
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "drmprimeout.hpp"

#include <boost/log/trivial.hpp>

#define ERRSTR strerror(errno)

void DRMPrimeOut::da_uninit(drm_aux_t* da)
{
	unsigned int i;

	if (da->framebufferHandle != 0) {
		drmModeRmFB(drmFd, da->framebufferHandle);
		da->framebufferHandle = 0;
	}

	for (i = 0; i != AV_DRM_MAX_PLANES; ++i) {
		if (da->boHandles[i]) {
			struct drm_gem_close gem_close = { .handle = da->boHandles[i] };
			drmIoctl(drmFd, DRM_IOCTL_GEM_CLOSE, &gem_close);
			da->boHandles[i] = 0;
		}
	}

	av_frame_free(&da->frame);
}

int DRMPrimeOut::renderFrame(AVFrame* frame)
{
	const AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)frame->data[0];
	drm_aux_t* da = aux + ano;
	const uint32_t format = desc->layers[0].format;
	int ret = 0;

	if (connectionData.outputFourCC != format) {
		bool foundPlane = DRMHelper::findPlane(connectionData.crtcIndex, format, &connectionData.planeId, planeNumber);
		if (! foundPlane) {
			av_frame_free(&frame);
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] No plane found that supports format " << DRMHelper::planeformatString(format);
			return -1;
		}
		connectionData.outputFourCC = format;
	}

	DRMHelper::waitVBlank();

	da_uninit(da);

	{
		uint32_t pitches[4] = { 0 };
		uint32_t offsets[4] = { 0 };
		uint64_t modifiers[4] = { 0 };
		uint32_t bo_handles[4] = { 0 };
		int i, j, n;

		da->frame = frame;

		memset(da->boHandles, 0, sizeof(da->boHandles));
		for (i = 0; i < desc->nb_objects; ++i) {
			if (drmPrimeFDToHandle(drmFd, desc->objects[i].fd, da->boHandles + i) != 0) {
				BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmPrimeFDToHandle failed:", ERRSTR;
				return -1;
			}
		}

		n = 0;
		for (i = 0; i < desc->nb_layers; ++i) {
			for (j = 0; j < desc->layers[i].nb_planes; ++j) {
				const AVDRMPlaneDescriptor* const p = desc->layers[i].planes + j;
				const AVDRMObjectDescriptor* const obj = desc->objects + p->object_index;
				pitches[n] = p->pitch;
				offsets[n] = p->offset;
				modifiers[n] = obj->format_modifier;
				bo_handles[n] = da->boHandles[p->object_index];
				++n;
			}
		}

		if (drmModeAddFB2WithModifiers(drmFd,
			av_frame_cropped_width(frame),
			av_frame_cropped_height(frame),
			desc->layers[0].format, bo_handles,
			pitches, offsets, modifiers,
			&da->framebufferHandle, DRM_MODE_FB_MODIFIERS /** 0 if no mods */) != 0) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeAddFB2WithModifiers failed: ", ERRSTR;
			return -1;
		}
	}

	ret = drmModeSetPlane(drmFd, connectionData.planeId, connectionData.crtcId,
		da->framebufferHandle, 0,
		compositionGeometry.x, compositionGeometry.y,
		compositionGeometry.width, compositionGeometry.height,
		0, 0,
		av_frame_cropped_width(frame) << 16,
		av_frame_cropped_height(frame) << 16);

	if (ret != 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeSetPlane failed:" << ERRSTR;
	}

	ano = ano + 1 >= AUX_SIZE ? 0 : ano + 1;

	return ret;
}


int DRMPrimeOut::displayFrame(struct AVFrame* src_frame)
{
	AVFrame* frame;
	int ret;

	if ((src_frame->flags & AV_FRAME_FLAG_CORRUPT) != 0) {
		BOOST_LOG_TRIVIAL(debug) << "[drmprime_out] discard corrupt frame: fmt=" << src_frame->format << ", ts = " << src_frame->pts;
		return 0;
	}

	if (src_frame->format == AV_PIX_FMT_DRM_PRIME) {
		frame = av_frame_alloc();
		av_frame_ref(frame, src_frame);
	}
	else if (src_frame->format == AV_PIX_FMT_VAAPI) {
		frame = av_frame_alloc();
		frame->format = AV_PIX_FMT_DRM_PRIME;
		if (av_hwframe_map(frame, src_frame, 0) != 0) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] failed to map frame (format=" << src_frame->format << ") to DRM_PRiME";
			av_frame_free(&frame);
			return AVERROR(EINVAL);
		}
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] frame (format=" << src_frame->format << ") not DRM_PRiME";
		return AVERROR(EINVAL);
	}

	// wait until the last frame has been processed?
	bool readyForNextFrame = true;
	if (show_all) {
		semaphoreRendererReady.wait();
	}
	else {
		readyForNextFrame = semaphoreRendererReady.try_wait();
	}

	if (readyForNextFrame) {
		nextFrame = frame;
		semaphoreNextFrameReady.post();
	}
	else {
		// drop frame
		av_frame_free(&frame);
	}

	return 0;
}

DRMPrimeOut::DRMPrimeOut(const CompositionGeometry compositionGeometry, int screenNumber, int planeNumber)
{
	drmFd = DRMHelper::getDRMDeviceFd(); // Cache it
	if (!drmHelper.initFullscreen(screenNumber)) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] failed to initialize display on screen " << screenNumber;
		terminate = true;
		return;
	}

	terminate = false;
	show_all = 1;
	this->planeNumber = planeNumber;

	connectionData = drmHelper.getConnectionData(screenNumber);
	if (! connectionData.connected) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] failed to to initialize display";
		terminate = true;
		return;
	}

	this->compositionGeometry = compositionGeometry;
	this->compositionGeometry.fitInto(connectionData.fullscreenGeometry);

	renderThread = thread(&DRMPrimeOut::renderLoop, this);
}

DRMPrimeOut::~DRMPrimeOut() {
	terminate = true;

	semaphoreNextFrameReady.post();
	renderThread.join();

	av_frame_free(&nextFrame);
}


void DRMPrimeOut::renderLoop()
{
	int i;

	semaphoreRendererReady.post();

	for (;;) {
		AVFrame* frame;

		semaphoreNextFrameReady.wait();

		if (terminate)
			break;

		frame = nextFrame;
		nextFrame = NULL;

		semaphoreRendererReady.post();

		renderFrame(frame);
	}

	for (i = 0; i != AUX_SIZE; ++i)
		da_uninit(aux + i);

	av_frame_free(&nextFrame);
}


