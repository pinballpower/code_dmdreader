#include <fcntl.h>

#include <boost/log/trivial.hpp>

#include "drmhelper.hpp"

// DRMHelper drmHelper; // singleton DRMHelper object

const vector<string> devicesToTry = { "/dev/dri/card0","/dev/dri/card1" };

// static variables of DRMHelper
int DRMHelper::drmDeviceFd = 0; // there is only a single file descriptor, even when using multiple screens
string DRMHelper::deviceFilename;
map<int, shared_ptr<DRMHelper>> DRMHelper::displayToDRM;
set<uint32_t> DRMHelper::planesInUse;


DRMException::DRMException(string message) {
	this->message = message;
}

const char* DRMException::what() const throw ()
{
	return "DRM Exception";
}


shared_ptr<DRMHelper> DRMHelper::getDRMForDisplay(int displayNumber) {
	if (displayToDRM.contains(displayNumber)) {
		return displayToDRM[displayNumber];
	}
	else {
		shared_ptr<DRMHelper> drmHelper(new DRMHelper());
		drmHelper->openDRMDevice();
		if (!drmHelper->initFullscreen(displayNumber)) {
			BOOST_LOG_TRIVIAL(debug) << "[drmhelper] culd not connect to display " << displayNumber;
			std::shared_ptr<DRMHelper> res(nullptr);
			return res;
		}
		displayToDRM[displayNumber] = drmHelper;
		return drmHelper;
	}
}

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
	if (DRMHelper::isOpen()) {
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
}

int DRMHelper::getDRMDeviceFd(bool autoOpen)
{
	if (autoOpen && (drmDeviceFd <= 0)) {
		openDRMDevice();
	}
	return drmDeviceFd;
}

bool DRMHelper::isOpen()
{
	return drmDeviceFd > 0;
}



const ScreenSize DRMHelper::getScreenSize() const
{
	return currentScreenSize;
}

const string DRMHelper::getDRMDeviceFilename()
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
	drmModeRmFB(drmDeviceFd, fb);
}

void DRMHelper::setPreviousCrtc()
{
	drmModeSetCrtc(drmDeviceFd, drmCrtc->crtc_id, drmCrtc->buffer_id, drmCrtc->x, drmCrtc->y, &drmConnectorId, 1, &drmCrtc->mode);
	drmModeFreeCrtc(drmCrtc);
}

void DRMHelper::usePlane(uint32_t planeId)
{
	planesInUse.insert(planeId);
}

void DRMHelper::unusePlane(uint32_t planeId)
{
	planesInUse.erase(planeId);
}

bool DRMHelper::isPlaneInUse(uint32_t planeId)
{
	return planesInUse.contains(planeId);
}

//
// C helper wrappers
//
extern "C" int cgetDRMDeviceFdForDisplay(int displayNumber)
{
	// do not initialize a display if only access to the DRM device is needed
	if (displayNumber < 0) {
		return DRMHelper::getDRMDeviceFd(true);
	}

	shared_ptr<DRMHelper> drmHelper = DRMHelper::getDRMForDisplay(displayNumber);
	if (drmHelper == nullptr) {
		return 0;
	}
	
	return DRMHelper::getDRMDeviceFd();
}

extern "C" int cgetDRMDeviceFd() {
	return cgetDRMDeviceFdForDisplay(-1);
}


bool DRMHelper::findCRTC(struct drm_setup* s, uint32_t* const pConId, int screenNumber)
{
	int i;
	bool returnCode = true;
	drmModeRes* res = drmModeGetResources(DRMHelper::drmDeviceFd);
	drmModeConnector* c;
	int currentScreen = 0;

	if (!res) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetResources failed";
		return false;
	}

	if (res->count_crtcs <= 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] no crts";
		returnCode = false;
		goto fail_res;
	}

	if (!s->connectionId) {
		BOOST_LOG_TRIVIAL(info) << "[drmprime_out] no connector ID specified, choosing default";

		for (i = 0; i < res->count_connectors; i++) {
			drmModeConnector* con =
				drmModeGetConnector(DRMHelper::drmDeviceFd, res->connectors[i]);
			drmModeEncoder* enc = NULL;
			drmModeCrtc* crtc = NULL;

			if (con->encoder_id) {
				enc = drmModeGetEncoder(DRMHelper::drmDeviceFd, con->encoder_id);
				if (enc->crtc_id) {
					crtc = drmModeGetCrtc(DRMHelper::drmDeviceFd, enc->crtc_id);
				}
			}

			string usingMsg = "";
			if (!s->connectionId && crtc) {
				if (screenNumber == currentScreen) {
					s->connectionId = con->connector_id;
					s->crtcId = crtc->crtc_id;
					usingMsg = "(selected)";
				}
				else {
					currentScreen++;
				}
				BOOST_LOG_TRIVIAL(info) << "[drmprime_out] connector " << con->connector_id << "(crtc " << crtc->crtc_id <<
					"): type " << con->connector_type << ": " << crtc->width << "x" << crtc->height << " " << usingMsg;
			}
		}

		if (!s->connectionId) {
			BOOST_LOG_TRIVIAL(error) << "[drmprime_out] no suitable enabled connector found";
			return false;
		}
	}

	s->crtcIndex = -1;

	for (i = 0; i < res->count_crtcs; ++i) {
		if (s->crtcId == res->crtcs[i]) {
			s->crtcIndex = i;
			break;
		}
	}

	if (s->crtcIndex == -1) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drm: CRTC " << s->crtcId << " not found";
		returnCode = false;
		goto fail_res;
	}

	if (res->count_connectors <= 0) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drm: no connectors";
		returnCode = false;
		goto fail_res;
	}

	c = drmModeGetConnector(DRMHelper::drmDeviceFd, s->connectionId);
	if (!c) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] drmModeGetConnector failed";
		returnCode = false;
		goto fail_res;
	}

	if (!c->count_modes) {
		BOOST_LOG_TRIVIAL(error) << "[drmprime_out] connector supports no mode";
		returnCode = false;
		goto fail_conn;
	}

	{
		drmModeCrtc* crtc = drmModeGetCrtc(DRMHelper::drmDeviceFd, s->crtcId);
		s->compose.x = crtc->x;
		s->compose.y = crtc->y;
		s->compose.width = crtc->width;
		s->compose.height = crtc->height;
		drmModeFreeCrtc(crtc);
	}


	if (pConId) *pConId = c->connector_id;
	returnCode = true;

fail_conn:
	drmModeFreeConnector(c);

fail_res:
	drmModeFreeResources(res);

	return returnCode;
}