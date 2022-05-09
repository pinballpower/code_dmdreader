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

struct compose_t {
	int x, y, width, height;
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
	/// <summary>
	/// Get the DRMHelper for one display.
	/// If this display has been initialized already, the object is returned. 
	/// Otherwise, the function will open the /dev/dri/... device, try to find the
	/// display with the given number and will initialize this display in fullscreen
	/// </summary>
	/// <param name="displayNumber">The number of the display. First display is number 0.</param>
	/// <returns></returns>
	static shared_ptr<DRMHelper> getDRMForDisplay(int displayNumber);

	static int getDRMDeviceFd(bool autoOpen = true);
	static bool isOpen();
	static const string getDRMDeviceFilename();

	bool initFullscreen(int displayNumber);
	const ScreenSize getScreenSize() const;

	uint32_t addAndActivateFramebuffer(uint32_t pitch, uint32_t handle);
	void removeFramebuffer(uint32_t fb);

	void setPreviousCrtc();

protected:

	static int drmDeviceFd;
	static bool openDRMDevice();
	static void closeDRMDevice();

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

// TODO: Refacturing

struct drm_setup
{
	int conId;
	uint32_t crtcId;
	int crtcIdx;
	uint32_t planeId;
	unsigned int out_fourcc;
	compose_t compose;
};

int find_crtc(int drmfd, struct drm_setup* s, uint32_t* const pConId, compose_t compose);