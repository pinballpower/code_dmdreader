#pragma once

#include <string>
#include <exception>
#include <map>
#include <memory>

#include <xf86drm.h>
#include <xf86drmMode.h>

using namespace std;


struct ScreenSize {
	int width;
	int height;
};

class DRMException : public std::exception
{

public:
	DRMException(string message);
	string message;

	const char* what() const throw () override;
};

class DRMHelper {

public:

	static shared_ptr<DRMHelper> getDRMForDisplay(int displayNumber);

	static bool openDRMDevice();
	static void closeDRMDevice();
	static int getDRMDeviceFd(bool autoInit = true);
	static bool isOpen();
	static const string getDRMDeviceFilename();

	bool initFullscreen(int displayNumber);
	const ScreenSize getScreenSize() const;

	uint32_t addAndActivateFramebuffer(uint32_t pitch, uint32_t handle);
	void removeFramebuffer(uint32_t fb);

	void setPreviousCrtc();

protected:

	static int drmDeviceFd;

private: 
	drmModeConnector* getDRMConnector(drmModeRes* resources, int displayNumber = 0);
	drmModeEncoder* findDRMEncoder(drmModeConnector* connector);

	static string deviceFilename;

	drmModeCrtc* drmCrtc = nullptr;
	uint32_t drmConnectorId = 0;
	bool initialized = false;
	ScreenSize currentScreenSize;
	drmModeModeInfo drmMode;

	static map<int, shared_ptr<DRMHelper>> displayToDRM;
};

extern "C" int cgetDRMDeviceFd();

// extern DRMHelper drmHelper;