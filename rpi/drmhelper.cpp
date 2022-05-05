#include <fcntl.h>

#include <boost/log/trivial.hpp>

#include "drmhelper.h"

static int drmDeviceFd;
drmModeModeInfo drmMode;
drmModeCrtc* drmCrtc;
uint32_t drmConnectorId;

static ScreenSize currentScreenSize;

drmModeConnector* getDRMConnector(drmModeRes* resources, int displayNumber)
{
	int currentDisplay = 0;

	for (int i = 0; i < resources->count_connectors; i++)
	{
		drmModeConnector* connector = drmModeGetConnector(drmDeviceFd, resources->connectors[i]);
		if (connector->connection == DRM_MODE_CONNECTED)
		{
			if (currentDisplay == displayNumber) {
				return connector;
			}
			else {
				currentDisplay++;
			}
		}
		drmModeFreeConnector(connector);
	}

	return NULL;
}

drmModeEncoder* findDRMEncoder(drmModeConnector* connector)
{
	if (connector->encoder_id)
	{
		return drmModeGetEncoder(drmDeviceFd, connector->encoder_id);
	}
	return NULL;
}

bool initDRM(int displayNumber) {
	drmModeRes* resources = drmModeGetResources(drmDeviceFd);
	if (resources == NULL)
	{
		BOOST_LOG_TRIVIAL(info) << "[pi4renderer] unable to get DRM resources";
		return false;
	}

	drmModeConnector* connector = getDRMConnector(resources, displayNumber);
	if (connector == NULL)
	{
		BOOST_LOG_TRIVIAL(debug) << "[pi4renderer] unable to get connector";
		drmModeFreeResources(resources);
		return false;
	}

	drmConnectorId = connector->connector_id;
	for (int i = 0; i < connector->count_modes; i++) {
		drmMode = connector->modes[i];
		BOOST_LOG_TRIVIAL(info) << "[pi4renderer] found supported resolution: " << drmMode.hdisplay << "x" << drmMode.vdisplay;
	}
	drmMode = connector->modes[0];
	currentScreenSize.width = drmMode.hdisplay;
	currentScreenSize.height = drmMode.vdisplay;

	BOOST_LOG_TRIVIAL(info) << "[pi4renderer] using native resolution: " << drmMode.hdisplay << "x" << drmMode.vdisplay;

	drmModeEncoder* encoder = findDRMEncoder(connector);
	if (encoder == NULL)
	{
		BOOST_LOG_TRIVIAL(info) << "[pi4renderer] unable to get encoder";
		drmModeFreeConnector(connector);
		drmModeFreeResources(resources);
		return false;
	}

	drmCrtc = drmModeGetCrtc(drmDeviceFd, encoder->crtc_id);
	drmModeFreeEncoder(encoder);
	drmModeFreeConnector(connector);
	drmModeFreeResources(resources);

	return true;
}

bool openDRMDevice(const string filename) {
	drmDeviceFd = open(filename.c_str(), O_RDWR | O_CLOEXEC);

	return true;
}

void closeDRMDevice() {
	if (drmDeviceFd) {
		close(drmDeviceFd);
	}
}

int getDRMDeviceFd()
{
	return drmDeviceFd;
}

const ScreenSize getScreenSize()
{
	return ScreenSize();
}
