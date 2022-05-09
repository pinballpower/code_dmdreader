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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_drm.h"
#include "libavutil/pixdesc.h"

#include "drmhelper.h" 
}


#include <boost/log/trivial.hpp>



#define DRM_MODULE "vc4"

#define ERRSTR strerror(errno)

struct drm_setup
{
	int conId;
	uint32_t crtcId;
	int crtcIdx;
	uint32_t planeId;
	unsigned int out_fourcc;
	compose_t compose;
};

typedef struct drm_aux_s
{
	unsigned int fb_handle;
	uint32_t bo_handles[AV_DRM_MAX_PLANES];

	AVFrame* frame;
} drm_aux_t;

// Aux size should only need to be 2, but on a few streams (Hobbit) under FKMS
// we get initial flicker probably due to dodgy drm timing
#define AUX_SIZE 3
typedef struct drmprime_out_env_s
{
	AVClass* avClass;

	int drm_fd;
	uint32_t con_id;
	struct drm_setup setup;
	enum AVPixelFormat avfmt;
	int show_all;

	unsigned int ano;
	drm_aux_t aux[AUX_SIZE];

	pthread_t q_thread;
	sem_t q_sem_in;
	sem_t q_sem_out;
	int q_terminate;
	AVFrame* q_next;

} drmprime_out_env_t;


static int find_plane(const int drmfd, const int crtcidx, const uint32_t format,
	uint32_t* const pplane_id)
{
	drmModePlaneResPtr planes;
	drmModePlanePtr plane;
	unsigned int i;
	unsigned int j;
	int ret = 0;

	planes = drmModeGetPlaneResources(drmfd);
	if (!planes) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetPlaneResources failed: " << ERRSTR;
		return -1;
	}

	for (i = 0; i < planes->count_planes; ++i) {
		plane = drmModeGetPlane(drmfd, planes->planes[i]);
		if (!planes) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetPlane failed: " << ERRSTR;
			break;
		}

		if (!(plane->possible_crtcs & (1 << crtcidx))) {
			drmModeFreePlane(plane);
			continue;
		}

		for (j = 0; j < plane->count_formats; ++j) {
			if (plane->formats[j] == format) break;
		}

		if (j == plane->count_formats) {
			drmModeFreePlane(plane);
			continue;
		}

		*pplane_id = plane->plane_id;
		drmModeFreePlane(plane);
		break;
	}

	if (i == planes->count_planes) ret = -1;

	drmModeFreePlaneResources(planes);
	return ret;
}

static void da_uninit(drmprime_out_env_t* const de, drm_aux_t* da)
{
	unsigned int i;

	if (da->fb_handle != 0) {
		drmModeRmFB(de->drm_fd, da->fb_handle);
		da->fb_handle = 0;
	}

	for (i = 0; i != AV_DRM_MAX_PLANES; ++i) {
		if (da->bo_handles[i]) {
			struct drm_gem_close gem_close = { .handle = da->bo_handles[i] };
			drmIoctl(de->drm_fd, DRM_IOCTL_GEM_CLOSE, &gem_close);
			da->bo_handles[i] = 0;
		}
	}

	av_frame_free(&da->frame);
}

static int do_display(drmprime_out_env_t* const de, AVFrame* frame)
{
	const AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)frame->data[0];
	drm_aux_t* da = de->aux + de->ano;
	const uint32_t format = desc->layers[0].format;
	int ret = 0;

	if (de->setup.out_fourcc != format) {
		if (find_plane(de->drm_fd, de->setup.crtcIdx, format, &de->setup.planeId)) {
			av_frame_free(&frame);
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] No plane for format " << format;
			return -1;
		}
		de->setup.out_fourcc = format;
	}

	{
		drmVBlank vbl = {
			.request = {
				.type = DRM_VBLANK_RELATIVE,
				.sequence = 0
			}
		};

		while (drmWaitVBlank(de->drm_fd, &vbl)) {
			if (errno != EINTR) {
				// This always fails - don't know why
				//                fprintf(stderr, "drmWaitVBlank failed: %s\n", ERRSTR);
				break;
			}
		}
	}

	da_uninit(de, da);

	{
		uint32_t pitches[4] = { 0 };
		uint32_t offsets[4] = { 0 };
		uint64_t modifiers[4] = { 0 };
		uint32_t bo_handles[4] = { 0 };
		int i, j, n;

		da->frame = frame;

		memset(da->bo_handles, 0, sizeof(da->bo_handles));
		for (i = 0; i < desc->nb_objects; ++i) {
			if (drmPrimeFDToHandle(de->drm_fd, desc->objects[i].fd, da->bo_handles + i) != 0) {
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
				bo_handles[n] = da->bo_handles[p->object_index];
				++n;
			}
		}

		if (drmModeAddFB2WithModifiers(de->drm_fd,
			av_frame_cropped_width(frame),
			av_frame_cropped_height(frame),
			desc->layers[0].format, bo_handles,
			pitches, offsets, modifiers,
			&da->fb_handle, DRM_MODE_FB_MODIFIERS /** 0 if no mods */) != 0) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeAddFB2WithModifiers failed: ", ERRSTR;
			return -1;
		}
	}

	ret = drmModeSetPlane(de->drm_fd, de->setup.planeId, de->setup.crtcId,
		da->fb_handle, 0,
		de->setup.compose.x, de->setup.compose.y,
		de->setup.compose.width,
		de->setup.compose.height,
		0, 0,
		av_frame_cropped_width(frame) << 16,
		av_frame_cropped_height(frame) << 16);

	if (ret != 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeSetPlane failed:" << ERRSTR;
	}

	de->ano = de->ano + 1 >= AUX_SIZE ? 0 : de->ano + 1;

	return ret;
}

static int do_sem_wait(sem_t* const sem, const int nowait)
{
	while (nowait ? sem_trywait(sem) : sem_wait(sem)) {
		if (errno != EINTR) return -errno;
	}
	return 0;
}

static void* display_thread(void* v)
{
	drmprime_out_env_t* const de = (drmprime_out_env_t*)v;
	int i;

	sem_post(&de->q_sem_out);

	for (;;) {
		AVFrame* frame;

		do_sem_wait(&de->q_sem_in, 0);

		if (de->q_terminate)
			break;

		frame = de->q_next;
		de->q_next = NULL;
		sem_post(&de->q_sem_out);

		do_display(de, frame);
	}

	for (i = 0; i != AUX_SIZE; ++i)
		da_uninit(de, de->aux + i);

	av_frame_free(&de->q_next);

	return NULL;
}

static int find_crtc(int drmfd, struct drm_setup* s, uint32_t* const pConId, compose_t compose)
{
	int ret = -1;
	int i;
	drmModeRes* res = drmModeGetResources(drmfd);
	drmModeConnector* c;

	if (!res) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetResources failed: " << ERRSTR;
		return -1;
	}

	if (res->count_crtcs <= 0) {
		printf("drm: no crts\n");
		goto fail_res;
	}

	if (!s->conId) {
		BOOST_LOG_TRIVIAL(info) << "[drmprime_out] no connector ID specified, choosing default";

		for (i = 0; i < res->count_connectors; i++) {
			drmModeConnector* con =
				drmModeGetConnector(drmfd, res->connectors[i]);
			drmModeEncoder* enc = NULL;
			drmModeCrtc* crtc = NULL;

			if (con->encoder_id) {
				enc = drmModeGetEncoder(drmfd, con->encoder_id);
				if (enc->crtc_id) {
					crtc = drmModeGetCrtc(drmfd, enc->crtc_id);
				}
			}

			if (!s->conId && crtc) {
				s->conId = con->connector_id;
				s->crtcId = crtc->crtc_id;
			}

			if (crtc) {
				BOOST_LOG_TRIVIAL(info) << "[drmprime_out] connector " << con->connector_id << "(crtc " << crtc->crtc_id <<
					"): type " << con->connector_type << ": " << crtc->width << "x" << crtc->height;
			}
		}

		if (!s->conId) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] no suitable enabled connector found";
			return -1;;
		}
	}

	s->crtcIdx = -1;

	for (i = 0; i < res->count_crtcs; ++i) {
		if (s->crtcId == res->crtcs[i]) {
			s->crtcIdx = i;
			break;
		}
	}

	if (s->crtcIdx == -1) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drm: CRTC "<< s->crtcId << " not found";
		goto fail_res;
	}

	if (res->count_connectors <= 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drm: no connectors";
		goto fail_res;
	}

	c = drmModeGetConnector(drmfd, s->conId);
	if (!c) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetConnector failed: " << ERRSTR;
		goto fail_res;
	}

	if (!c->count_modes) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] connector supports no mode";
		goto fail_conn;
	}

	{
		drmModeCrtc* crtc = drmModeGetCrtc(drmfd, s->crtcId);
		if (compose.x < 0) {
			s->compose.x = crtc->x;
		}
		else {
			s->compose.x = compose.x;
		}
		if (compose.y < 0) {
			s->compose.y = crtc->y;
		}
		else {
			s->compose.y = compose.y;
		}
		if (compose.width < 0) {
			s->compose.width = crtc->width;
		}
		else {
			s->compose.width = compose.width;
		}
		if (compose.height < 0) {
			s->compose.height = crtc->height;
		}
		else {
			s->compose.height = compose.height;
		}
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

int drmprime_out_display(drmprime_out_env_t* de, struct AVFrame* src_frame)
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
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] frame (format="<< src_frame->format << ") not DRM_PRiME";
		return AVERROR(EINVAL);
	}

	ret = do_sem_wait(&de->q_sem_out, !de->show_all);
	if (ret) {
		av_frame_free(&frame);
	}
	else {
		de->q_next = frame;
		sem_post(&de->q_sem_in);
	}

	return 0;
}

void drmprime_out_delete(drmprime_out_env_t* de)
{
	de->q_terminate = 1;
	sem_post(&de->q_sem_in);
	pthread_join(de->q_thread, NULL);
	sem_destroy(&de->q_sem_in);
	sem_destroy(&de->q_sem_out);

	av_frame_free(&de->q_next);

	delete de;
}

drmprime_out_env_t* drmprime_out_new(compose_t compose)
{
	int rv;
	drmprime_out_env_t* const de = new drmprime_out_env_t;
	if (de == NULL)
		return NULL;

	const char* drm_module = DRM_MODULE;

	de->drm_fd = cgetDRMDeviceFd();
	de->con_id = 0;
	de->setup = (struct drm_setup){ 0 };
	de->q_terminate = 0;
	de->show_all = 1;

	if (find_crtc(de->drm_fd, &de->setup, &de->con_id, compose) != 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] failed to find valid mode";
		rv = AVERROR(EINVAL);
		goto fail_close;
	}


	sem_init(&de->q_sem_in, 0, 0);
	sem_init(&de->q_sem_out, 0, 0);
	if (pthread_create(&de->q_thread, NULL, display_thread, (void*)de)) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] failed to create display thread";
		goto fail_close;
	}

	return de;

fail_close:
	close(de->drm_fd);
	de->drm_fd = -1;
fail_free:
	delete de;
	fprintf(stderr, ">>> %s: FAIL\n", __func__);
	return NULL;
}
