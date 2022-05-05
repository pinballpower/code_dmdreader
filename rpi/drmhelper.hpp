#pragma once

#include <string>

#include <xf86drm.h>
#include <xf86drmMode.h>

using namespace std;


struct ScreenSize {
	int width;
	int height;
};

class DRMHelper {

public:

	bool openDRMDevice();
	void closeDRMDevice();
	int getDRMDeviceFd(bool autoInit = true);
	bool isOpen();

	bool initFullscreen(int displayNumber);
	const ScreenSize getScreenSize() const;
	const string getDRMDeviceFilename() const;

	uint32_t addAndActivateFramebuffer(uint32_t pitch, uint32_t handle);
	void removeFramebuffer(uint32_t fb);

	void setPreviousCrtc();

protected:

	static int drmDeviceFd;

private: 
	drmModeConnector* getDRMConnector(drmModeRes* resources, int displayNumber = 0);
	drmModeEncoder* findDRMEncoder(drmModeConnector* connector);

	drmModeCrtc* drmCrtc;
	uint32_t drmConnectorId;
	string deviceFilename;
	bool initialized = false;
	ScreenSize currentScreenSize;
	drmModeModeInfo drmMode;
};

extern "C" int cgetDRMDeviceFd();

extern DRMHelper drmHelper;