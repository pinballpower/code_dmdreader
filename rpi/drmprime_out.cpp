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

#include "drmprime_out.h"

#include <boost/log/trivial.hpp>

#define DRM_MODULE "vc4"

#define ERRSTR strerror(errno)

int findCRTC(int drmFd, struct drm_setup* s, uint32_t* const pConId, int screenNumber)
{
	int ret = -1;
	int i;
	drmModeRes* res = drmModeGetResources(drmFd);
	drmModeConnector* c;
	int currentScreen = 0;

	if (!res) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetResources failed";
		return -1;
	}

	if (res->count_crtcs <= 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] no crts";
		goto fail_res;
	}

	if (!s->connectionId) {
		BOOST_LOG_TRIVIAL(info) << "[drmprime_out] no connector ID specified, choosing default";

		for (i = 0; i < res->count_connectors; i++) {
			drmModeConnector* con =
				drmModeGetConnector(drmFd, res->connectors[i]);
			drmModeEncoder* enc = NULL;
			drmModeCrtc* crtc = NULL;

			if (con->encoder_id) {
				enc = drmModeGetEncoder(drmFd, con->encoder_id);
				if (enc->crtc_id) {
					crtc = drmModeGetCrtc(drmFd, enc->crtc_id);
				}
			}

			string usingMsg = "";
			if (!s->connectionId && crtc) {
				if (screenNumber == currentScreen) {
					s->connectionId = con->connector_id;
					s->crtcId = crtc->crtc_id;
					usingMsg = "(selected)";
				}
				else {
					currentScreen++;
				}
				BOOST_LOG_TRIVIAL(info) << "[drmprime_out] connector " << con->connector_id << "(crtc " << crtc->crtc_id <<
					"): type " << con->connector_type << ": " << crtc->width << "x" << crtc->height << " " << usingMsg;
			}
		}

		if (!s->connectionId) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] no suitable enabled connector found";
			return -1;;
		}
	}

	s->crtcIndex = -1;

	for (i = 0; i < res->count_crtcs; ++i) {
		if (s->crtcId == res->crtcs[i]) {
			s->crtcIndex = i;
			break;
		}
	}

	if (s->crtcIndex == -1) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drm: CRTC " << s->crtcId << " not found";
		goto fail_res;
	}

	if (res->count_connectors <= 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drm: no connectors";
		goto fail_res;
	}

	c = drmModeGetConnector(drmFd, s->connectionId);
	if (!c) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetConnector failed";
		goto fail_res;
	}

	if (!c->count_modes) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] connector supports no mode";
		goto fail_conn;
	}

	{
		drmModeCrtc* crtc = drmModeGetCrtc(drmFd, s->crtcId);
		s->compose.x = crtc->x;
		s->compose.y = crtc->y;
		s->compose.width = crtc->width;
		s->compose.height = crtc->height;
		drmModeFreeCrtc(crtc); 
	}


	if (pConId) *pConId = c->connector_id;
	ret = 0;

fail_conn:
	drmModeFreeConnector(c);

fail_res:
	drmModeFreeResources(res);

	return ret;
}

static int findPlane(const int drmFd, const int crtcIndex, const uint32_t format, uint32_t* const pplaneId, const int planeNumber)
{
	drmModePlaneResPtr planes;
	drmModePlanePtr plane;
	unsigned int i;
	unsigned int j;
	int ret = 0;
	int currentPlane = 0;

	planes = drmModeGetPlaneResources(drmFd);
	if (!planes) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetPlaneResources failed: " << ERRSTR;
		return -1;
	}

	for (i = 0; i < planes->count_planes; ++i) {
		plane = drmModeGetPlane(drmFd, planes->planes[i]);
		if (!planes) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetPlane failed: " << ERRSTR;
			break;
		}

		if (!(plane->possible_crtcs & (1 << crtcIndex))) {
			drmModeFreePlane(plane);
			continue;
		}

		for (j = 0; j < plane->count_formats; ++j) {
			if (plane->formats[j] == format) {
				if (currentPlane == planeNumber) {
					BOOST_LOG_TRIVIAL(error) << "[drmprime_out] plane " << currentPlane;
					break;
				}
				else {
					currentPlane++;
				}
			}
		}

		if (j == plane->count_formats) {
			drmModeFreePlane(plane);
			continue;
		}

		*pplaneId = plane->plane_id;		
		drmModeFreePlane(plane);
		break;
	}

	if (i == planes->count_planes) ret = -1;

	drmModeFreePlaneResources(planes);
	return ret;
}

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

	if (setup.out_fourcc != format) {
		if (findPlane(drmFd, setup.crtcIndex, format, &setup.planeId, planeNumber)) {
			av_frame_free(&frame);
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] No plane for format " << format;
			return -1;
		}
		setup.out_fourcc = format;
	}

	{
		drmVBlank vbl = {
			.request = {
				.type = DRM_VBLANK_RELATIVE,
				.sequence = 0
			}
		};

		while (drmWaitVBlank(drmFd, &vbl)) {
			if (errno != EINTR) {
				// This always fails - don't know why
				//                fprintf(stderr, "drmWaitVBlank failed: %s\n", ERRSTR);
				break;
			}
		}
	}

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

	ret = drmModeSetPlane(drmFd, setup.planeId, setup.crtcId,
		da->framebufferHandle, 0,
		setup.compose.x, setup.compose.y,
		setup.compose.width, setup.compose.height,
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
		q_next = frame;
		semaphoreNextFrameReady.post();
	}
	else {
		// drop frame
		av_frame_free(&frame);
	}

	return 0;
}

DRMPrimeOut::DRMPrimeOut(compose_t compose, int screenNumber, int planeNumber)
{
	int rv;

	const char* drm_module = DRM_MODULE;

	drmFd = DRMHelper::getDRMDeviceFd(); // TODO: Move some parts of this into drmhelper
	con_id = 0;
	setup = (struct drm_setup){ 0 };
	terminate = false;
	show_all = 1;
	this->planeNumber = planeNumber;

	if (findCRTC(drmFd, &setup, &con_id, screenNumber) != 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] failed to find valid mode";
	}

	// override fullscreen if compose values are given
	if (compose.x >= 0) {
		setup.compose.x = compose.x;
	}
	if (compose.y >= 0) {
		setup.compose.y = compose.y;
	}
	if (compose.width >= 0) {
		setup.compose.width = compose.width;
	}
	if (compose.height >= 0) {
		setup.compose.height = compose.height;
	}

	renderThread = thread(&DRMPrimeOut::renderLoop, this);
}

DRMPrimeOut::~DRMPrimeOut() {
	terminate = true;

	semaphoreNextFrameReady.post();
	renderThread.join();

	av_frame_free(&q_next);
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

		frame = q_next;
		q_next = NULL;

		semaphoreRendererReady.post();

		renderFrame(frame);
	}

	for (i = 0; i != AUX_SIZE; ++i)
		da_uninit(aux + i);

	av_frame_free(&q_next);
}


