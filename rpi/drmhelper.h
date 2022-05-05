#pragma once

#include <string>

#include <xf86drm.h>
#include <xf86drmMode.h>

extern drmModeModeInfo drmMode;
extern drmModeCrtc* drmCrtc;
extern uint32_t drmConnectorId;

using namespace std;

bool initDRM(int displayNumber);
bool openDRMDevice(const string filename);
void closeDRMDevice();
int getDRMDeviceFd();

drmModeConnector* getDRMConnector(drmModeRes* resources, int displayNumber = 0);
drmModeEncoder* findDRMEncoder(drmModeConnector* connector);

