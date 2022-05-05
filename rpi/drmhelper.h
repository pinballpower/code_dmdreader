#pragma once

#include <xf86drm.h>
#include <xf86drmMode.h>

extern int drmDeviceFd;
extern drmModeModeInfo drmMode;
extern drmModeCrtc* drmCrtc;
extern uint32_t drmConnectorId;

drmModeConnector* getDRMConnector(int drmDeviceFd, drmModeRes* resources, int displayNumber = 0);
drmModeEncoder* findDRMEncoder(int drmDeviceFd, drmModeConnector* connector);