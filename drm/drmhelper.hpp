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

class CompositionGeometry {

public: 

	CompositionGeometry(int x = -1, int y = -1, int width = -1, int height = -1);

	int x = -1;
	int y = -1;
	int width = -1;
	int height = -1;

	void fitInto(const CompositionGeometry screenSize);
	bool isUndefined();
};

struct DRMConnectionData
{
	bool connected = false;
	int connectionId = 0;
	uint32_t crtcId = 0;
	int crtcIndex = 0;
	uint32_t planeId = 0;
	int connectorWidth = 0;
	int connectorHeight = 0;
	/// <summary>
	/// This is a 4-character pixel format encoded in a 32bit work, e.g. '21VN'-> 0x3231564E
	/// </summary>
	uint32_t outputFourCC = 0;
	CompositionGeometry fullscreenGeometry;
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

	bool initFullscreen(int displayNumber, int width=0, int height=0);
	static CompositionGeometry getFullscreenResolution(int displayNumber);

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

	static void waitVBlank();

	static string planeformatString(uint32_t format);
	static uint32_t planeformat(string formatString);
	static bool setAlphaForPlane(uint32_t planeId, uint32_t alpha);
	static bool setPixelBlendCoverageForPlane(uint32_t planeId);

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

