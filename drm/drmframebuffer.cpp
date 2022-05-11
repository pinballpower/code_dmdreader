#include "drmframebuffer.hpp"

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <boost/log/trivial.hpp>

struct framebuffer {
	int fd;
	uint32_t buffer_id;
	uint16_t res_x;
	uint16_t res_y;
	uint8_t* data;
	uint32_t size;
	struct drm_mode_create_dumb dumb_framebuffer;
	drmModeCrtcPtr crtc;
	drmModeConnectorPtr connector;
	drmModeModeInfoPtr resolution;
};


DRMFrameBuffer::DRMFrameBuffer(int screenNumber, int planeNumber)
{
	drmModeConnector* connector = nullptr;
	drmModeEncoder* encoder = nullptr;
	drmModeCrtc* crtc_to_restore = nullptr;
	uint8_t* framebufferData = nullptr;
	int framebufferLen = 100 * 100 * 4;

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

	framebufferLen = connectionData.compositionGeometry.width * connectionData.compositionGeometry.height * 4;
	framebufferData = (uint8_t*)mmap(0, framebufferLen, PROT_READ | PROT_WRITE, MAP_SHARED, drmFd, mreq.offset);
	if (framebufferData == MAP_FAILED) {
		err = errno;
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] mode map failed, err=" << err;
		framebufferData = nullptr;
		goto cleanup;
	}

	//{
	//	uint8_t* data = framebufferData;
	//	for (int i = 0; i < connectionData.connectorWidth * connectionData.connectorHeight; i++, data += 4) {
	//		data[0] = i & 0xff ; // b
	//		data[1] = 0x00; // g 
	//		data[2] = 0x00; // r
	//		data[3] = 0xff;    
	//	}
	//}

	memset(framebufferData, 0xff, framebufferLen);


	{
		uint32_t planeFormat = DRMHelper::planeformat("AR24");
		bool planeFound = DRMHelper::findPlane(connectionData.crtcIndex, planeFormat, &connectionData.planeId, 5);
		if (!planeFound) {
			BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] No plane found for format " << DRMHelper::planeformatString(planeFormat);
			goto cleanup;
		}
	}
	err = drmModeSetPlane(drmFd, connectionData.planeId, connectionData.crtcId,
		framebufferId, 0,
		100,100,
		200,200,
		0, 0,
		connectionData.compositionGeometry.height << 16,
		connectionData.compositionGeometry.height << 16);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] Can't connect framebuffer to plane: " << strerror(errno);
		goto cleanup;
	}

	/*

	ret = drmModeSetPlane(drmFd, connectionData.planeId, connectionData.crtcId,
		da->framebufferHandle, 0,
		connectionData.compositionGeometry.x, connectionData.compositionGeometry.y,
		connectionData.compositionGeometry.width, connectionData.compositionGeometry.height,
		0, 0,
		av_frame_cropped_width(frame) << 16,
		av_frame_cropped_height(frame) << 16);*/

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
