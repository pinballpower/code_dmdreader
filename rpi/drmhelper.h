#pragma once

#include <xf86drm.h>
#include <xf86drmMode.h>

drmModeConnector* getDRMConnector(int drmDeviceFd, drmModeRes* resources, int displayNumber = 0);
drmModeEncoder* findDRMEncoder(int drmDeviceFd, drmModeConnector* connector);