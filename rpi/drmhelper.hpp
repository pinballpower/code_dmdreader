#pragma once

#include <string>

#include <xf86drm.h>
#include <xf86drmMode.h>

using namespace std;


// TODO: move these into the object and make them private
extern drmModeModeInfo drmMode;
extern drmModeCrtc* drmCrtc;
extern uint32_t drmConnectorId;

struct ScreenSize {
	int width;
	int height;
};

class DRMHelper {

public:

	bool initDRM(int displayNumber);
	bool openDRMDevice();
	void closeDRMDevice();
	int getDRMDeviceFd(bool autoInit = true);

	const ScreenSize getScreenSize();
	const string getDRMDeviceFilename();

private: 
	drmModeConnector* getDRMConnector(drmModeRes* resources, int displayNumber = 0);
	drmModeEncoder* findDRMEncoder(drmModeConnector* connector);
};

extern "C" int cgetDRMDeviceFd();

extern DRMHelper drmHelper;