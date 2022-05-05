#pragma once

#include <string>

#include <xf86drm.h>
#include <xf86drmMode.h>

extern int drmDeviceFd;
extern drmModeModeInfo drmMode;
extern drmModeCrtc* drmCrtc;
extern uint32_t drmConnectorId;

using namespace std;

bool initDRM(int displayNumber);
bool openDRMDevice(string filename);
void closeDRMDevice();
drmModeConnector* getDRMConnector(int drmDeviceFd, drmModeRes* resources, int displayNumber = 0);
drmModeEncoder* findDRMEncoder(int drmDeviceFd, drmModeConnector* connector);

