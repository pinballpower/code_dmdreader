#include "drmframebuffer.hpp"

#include <sys/ioctl.h>

#include <boost/log/trivial.hpp>

DRMFrameBuffer::DRMFrameBuffer(int screenNumber, int planeNumber)
{
	drmModeConnector* connector = nullptr;
	drmModeEncoder* encoder = nullptr;

	int err;

	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;

	drmFd = DRMHelper::getDRMDeviceFd(); // Cache it
	connectionData = DRMHelper::getConnectionData(screenNumber);
	if (! connectionData.connected) {
		BOOST_LOG_TRIVIAL(error) << "[drmhelper] could not connect to screen " << screenNumber;
	}

	drm_mode_create_dumb dumbBufferConfig;
	dumbBufferConfig.width = connectionData.connectorWidth;
	dumbBufferConfig.height = connectionData.connectorHeight;
	dumbBufferConfig.bpp = 32;

	err = ioctl(drmFd, DRM_IOCTL_MODE_CREATE_DUMB, &dumbBufferConfig);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmhelper] could not create dumb buffer: " << strerror(errno);
	}

	uint32_t buffer_id;
	err = drmModeAddFB(drmFd,connectionData.compositionGeometry.width, connectionData.compositionGeometry.height, 24, 32,
		dumbBufferConfig.pitch, dumbBufferConfig.handle, &buffer_id);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmhelper] could not add framebuffer to drm: " << strerror(errno);
		goto cleanup;
	}

	connector = drmModeGetConnector(drmFd, connectionData.connectionId);
	encoder = drmModeGetEncoder(drmFd, connector->encoder_id);
	if (!encoder) {
		BOOST_LOG_TRIVIAL(error) << "[drmhelper] could not get encoder";
		goto cleanup;
	}

	err = 0;

cleanup:

	// cleanup connector
	// cleanup encoder
	// cleanup framebuffer

	err = 0;

}

DRMFrameBuffer::~DRMFrameBuffer()
{
}
