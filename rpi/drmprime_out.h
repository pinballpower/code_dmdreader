#pragma once 

#include <thread>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

extern "C" {
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_drm.h"
#include "libavutil/pixdesc.h"
}

#include "drmhelper.hpp"

using namespace std;
using namespace boost::interprocess;


// Aux size should only need to be 2, but on a few streams (Hobbit) under FKMS
// we get initial flicker probably due to dodgy drm timing
#define AUX_SIZE 3

struct drm_aux_t
{
	unsigned int framebufferHandle;
	uint32_t boHandles[AV_DRM_MAX_PLANES];

	AVFrame* frame = nullptr;
};

struct drm_setup
{
	int connectionId;
	uint32_t crtcId;
	int crtcIndex;
	uint32_t planeId;
	unsigned int out_fourcc;
	compose_t compose;
};


class DRMPrimeOut
{

public: 
	DRMPrimeOut(compose_t compose, int screenNumber=0);
	~DRMPrimeOut();

	int displayFrame(struct AVFrame* frame);


private:
	void renderLoop();
	void da_uninit(drm_aux_t* da);
	int renderFrame(AVFrame* frame);

	int drmFd;
	int screenNumber;
	uint32_t con_id;
	struct drm_setup setup;
	enum AVPixelFormat avfmt;
	int show_all;

	unsigned int ano;
	drm_aux_t aux[AUX_SIZE];

	thread renderThread;
	interprocess_semaphore semaphoreNextFrameReady = interprocess_semaphore(0);
	interprocess_semaphore semaphoreRendererReady = interprocess_semaphore(0);
	bool terminate = false;
	AVFrame* q_next;

};


