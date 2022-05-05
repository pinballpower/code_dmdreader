#include <fcntl.h>

#include <boost/log/trivial.hpp>

#include "drmhelper.hpp"

DRMHelper drmHelper; // singleton DRMHelper object

drmModeModeInfo drmMode;

const vector<string> devicesToTry = { "/dev/dri/card0","/dev/dri/card1" };

drmModeConnector* DRMHelper::getDRMConnector(drmModeRes* resources, int displayNumber)
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

drmModeEncoder* DRMHelper::findDRMEncoder(drmModeConnector* connector)
{
	if (connector->encoder_id)
	{
		return drmModeGetEncoder(drmDeviceFd, connector->encoder_id);
	}
	return NULL;
}

bool DRMHelper::initFullscreen(int displayNumber) {
	drmModeRes* resources = drmModeGetResources(drmDeviceFd);
	if (resources == NULL)
	{
		BOOST_LOG_TRIVIAL(info) << "[drmhelper] unable to get DRM resources";
		return false;
	}

	drmModeConnector* connector = getDRMConnector(resources, displayNumber);
	if (connector == NULL)
	{
		BOOST_LOG_TRIVIAL(debug) << "[drmhelper] unable to get connector";
		drmModeFreeResources(resources);
		return false;
	}

	drmConnectorId = connector->connector_id;
	for (int i = 0; i < connector->count_modes; i++) {
		drmMode = connector->modes[i];
		BOOST_LOG_TRIVIAL(info) << "[drmhelper] found supported resolution: " << drmMode.hdisplay << "x" << drmMode.vdisplay;
	}
	drmMode = connector->modes[0];
	currentScreenSize.width = drmMode.hdisplay;
	currentScreenSize.height = drmMode.vdisplay;

	BOOST_LOG_TRIVIAL(info) << "[drmhelper] using native resolution: " << drmMode.hdisplay << "x" << drmMode.vdisplay;

	drmModeEncoder* encoder = findDRMEncoder(connector);
	if (encoder == NULL)
	{
		BOOST_LOG_TRIVIAL(info) << "[drmhelper] unable to get encoder";
		drmModeFreeConnector(connector);
		drmModeFreeResources(resources);
		return false;
	}

	drmCrtc = drmModeGetCrtc(drmDeviceFd, encoder->crtc_id);
	drmModeFreeEncoder(encoder);
	drmModeFreeConnector(connector);
	drmModeFreeResources(resources);

	initialized = true;

	return true;
}

bool DRMHelper::openDRMDevice() {
	if (this->isOpen()) {
		return true;
	};

	for (auto filename : devicesToTry) {
		drmDeviceFd = open(filename.c_str(), O_RDWR | O_CLOEXEC);

		drmModeRes* resources = drmModeGetResources(drmDeviceFd);
		if (resources == NULL)
		{
			closeDRMDevice();
		}
		else {
			BOOST_LOG_TRIVIAL(info) << "[drmhelper] using DRM device " << filename;
			deviceFilename = filename;
			return true;
		}
	}

	return false;
}

void DRMHelper::closeDRMDevice() {
	if (drmDeviceFd) {
		close(drmDeviceFd);
	}
	drmDeviceFd = 0;
	deviceFilename = "";
	initialized = false;
}

int DRMHelper::getDRMDeviceFd(bool autoInit)
{
	if (drmDeviceFd <= 0) {
		this->openDRMDevice();
		return drmDeviceFd;
	}
	else {
		return drmDeviceFd;
	}
}

bool DRMHelper::isOpen()
{
	return drmDeviceFd > 0;
}

extern "C" int cgetDRMDeviceFd()
{
	return drmHelper.getDRMDeviceFd();
}

const ScreenSize DRMHelper::getScreenSize() const
{
	return ScreenSize();
}

const string DRMHelper::getDRMDeviceFilename() const
{
	return deviceFilename;
}

uint32_t DRMHelper::addAndActivateFramebuffer(uint32_t pitch, uint32_t handle) {
	uint32_t fb = 0;
	drmModeAddFB(drmDeviceFd, drmMode.hdisplay, drmMode.vdisplay, 24, 32, pitch, handle, &fb);
	drmModeSetCrtc(drmDeviceFd, drmCrtc->crtc_id, fb, 0, 0, &drmConnectorId, 1, &drmMode);
	return fb;
}

void DRMHelper::removeFramebuffer(uint32_t fb) {
	drmModeRmFB(drmHelper.getDRMDeviceFd(), fb);
}

void DRMHelper::setPreviousCrtc()
{
	drmModeSetCrtc(drmDeviceFd, drmCrtc->crtc_id, drmCrtc->buffer_id, drmCrtc->x, drmCrtc->y, &drmConnectorId, 1, &drmCrtc->mode);
	drmModeFreeCrtc(drmCrtc);
}
