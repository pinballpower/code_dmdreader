#include "drmhelper.h"

#include <boost/log/trivial.hpp>

int drmDeviceFd;
drmModeModeInfo drmMode;
drmModeCrtc* drmCrtc;
uint32_t drmConnectorId;

drmModeConnector* getDRMConnector(int drmDeviceFd, drmModeRes* resources, int displayNumber)
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

drmModeEncoder* findDRMEncoder(int drmDeviceFd, drmModeConnector* connector)
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

	drmModeConnector* connector = getDRMConnector(drmDeviceFd, resources, displayNumber);
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
	BOOST_LOG_TRIVIAL(info) << "[pi4renderer] using native resolution: " << drmMode.hdisplay << "x" << drmMode.vdisplay;

	drmModeEncoder* encoder = findDRMEncoder(drmDeviceFd, connector);
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