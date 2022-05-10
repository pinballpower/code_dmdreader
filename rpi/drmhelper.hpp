#pragma once

#include <string>
#include <exception>
#include <map>
#include <memory>
#include <set>

#include <xf86drm.h>
#include <xf86drmMode.h>

using namespace std;

struct ScreenSize {
	int width;
	int height;
};

struct CompositionGeometry {
	int x = -1;
	int y = -1;
	int width = -1;
	int height = -1;
};

struct DRMConnectionData
{
	bool connected = false;
	int connectionId = 0;
	uint32_t crtcId = 0;
	int crtcIndex = 0;
	uint32_t planeId = 0;
	/// <summary>
	/// This is a 4-character pixel format encoded in a 32bit work, e.g. '21VN'-> 0x3231564E
	/// </summary>
	uint32_t outputFourCC = 0;
	CompositionGeometry compositionGeometry;
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

	static void usePlane(uint32_t planeId);
	static void unusePlane(uint32_t planeId);
	static bool isPlaneInUse(uint32_t planeId);

	static DRMConnectionData getConnectionData(int displayNumber);
	static bool findPlane(const int crtcIndex, const uint32_t format, uint32_t* const pplaneId, const int planeNumber);

	static void logResources();

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
	static set<uint32_t> planesInUse;

	
};



