#include "drmframebuffer.hpp"

#include <sys/ioctl.h>

#include <boost/log/trivial.hpp>

DRMFrameBuffer::DRMFrameBuffer(int screenNumber, int planeNumber)
{
	drmModeConnector* connector = nullptr;
	drmModeEncoder* encoder = nullptr;
	drmModeCrtc* crtc_to_restore = nullptr;

	int err;

	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;

	drmFd = DRMHelper::getDRMDeviceFd(); // Cache it
	connectionData = DRMHelper::getConnectionData(screenNumber);
	if (! connectionData.connected) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not connect to screen " << screenNumber;
	}

	drm_mode_create_dumb dumbBufferConfig;
	dumbBufferConfig.width = connectionData.connectorWidth;
	dumbBufferConfig.height = connectionData.connectorHeight;
	dumbBufferConfig.bpp = 32;

	err = ioctl(drmFd, DRM_IOCTL_MODE_CREATE_DUMB, &dumbBufferConfig);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not create dumb buffer: " << strerror(errno);
	}

	uint32_t framebufferId;
	err = drmModeAddFB(drmFd,connectionData.compositionGeometry.width, connectionData.compositionGeometry.height, 24, 32,
		dumbBufferConfig.pitch, dumbBufferConfig.handle, &framebufferId);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not add framebuffer to drm: " << strerror(errno);
		goto cleanup;
	}

	connector = drmModeGetConnector(drmFd, connectionData.connectionId);
	encoder = drmModeGetEncoder(drmFd, connector->encoder_id);
	if (!encoder) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not get encoder";
		goto cleanup;
	}

	struct drm_mode_map_dumb mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = dumbBufferConfig.handle;

	err = drmIoctl(drmFd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] Mode map dumb framebuffer failed: error=" << err;
		goto cleanup;
	}

	/* Backup the informations of the CRTC to restore when we're done.
	 * The most important piece seems to currently be the buffer ID.
	 */
	crtc_to_restore =
		drmModeGetCrtc(drmFd, connectionData.crtcId);
	if (!crtc_to_restore) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not retrieve the current CRTC with a valid ID";
		goto cleanup;
	}

	/* Set the CRTC so that it uses our new framebuffer */
	err = drmModeSetCrtc(
		drmFd, connectionData.crtcId, framebufferId,
		0, 0,
		&connector->connector_id,
		1,
		&crtc_to_restore->mode);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not active screen: " << strerror(errno);
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
