#pragma once

#include <string>

#include <xf86drm.h>
#include <xf86drmMode.h>

extern drmModeModeInfo drmMode;
extern drmModeCrtc* drmCrtc;
extern uint32_t drmConnectorId;

struct ScreenSize {
	int width;
	int height;
};

using namespace std;

bool initDRM(int displayNumber);
bool openDRMDevice();
void closeDRMDevice();
int getDRMDeviceFd(bool autoInit=true);

extern "C" int cgetDRMDeviceFd();

const ScreenSize getScreenSize();
const string getDRMDeviceFilename();

drmModeConnector* getDRMConnector(drmModeRes* resources, int displayNumber = 0);
drmModeEncoder* findDRMEncoder(drmModeConnector* connector);

