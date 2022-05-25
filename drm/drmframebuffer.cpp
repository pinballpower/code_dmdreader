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



DRMFrameBuffer::DRMFrameBuffer(int screenNumber, int planeNumber, const CompositionGeometry geometry)
{
	int err;

	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;
	this->compositionGeometry = geometry;

	drmFd = DRMHelper::getDRMDeviceFd(); // Cache it
	connectionData = DRMHelper::getConnectionData(screenNumber);
	if (! connectionData.connected) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] couln't connect to screen " << screenNumber;
	}

	width = connectionData.connectorWidth;
	height = connectionData.connectorHeight;
	bytesPerLine = width * 4;

	drm_mode_create_dumb dumbBufferConfig;
	dumbBufferConfig.width = width;
	dumbBufferConfig.height = height;
	dumbBufferConfig.bpp = 32;

	err = ioctl(drmFd, DRM_IOCTL_MODE_CREATE_DUMB, &dumbBufferConfig);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not create dumb buffer: " << strerror(errno);
	}

	uint32_t framebufferId;
	// frame buffer size has to match the resolution of the connector!
	// also make sure, depth is 32 as we use an alpha channel
	err = drmModeAddFB(drmFd, width, height, 32, 32,
		dumbBufferConfig.pitch, dumbBufferConfig.handle, &framebufferId);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] couldn't add framebuffer to drm: " << strerror(errno);
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

	this->compositionGeometry.fitInto(connectionData.fullscreenGeometry);

	framebufferLen = width * height * 4;
	framebufferData = (uint8_t*)mmap(0, framebufferLen, PROT_READ | PROT_WRITE, MAP_SHARED, drmFd, mreq.offset);
	if (framebufferData == MAP_FAILED) {
		err = errno;
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] mode map failed, err=" << err;
		framebufferData = nullptr;
		framebufferLen = 0;
		goto cleanup;
	}

	{
		uint32_t planeFormat = DRMHelper::planeformat("AR24");
		bool planeFound = DRMHelper::findPlane(connectionData.crtcIndex, planeFormat, &planeId, planeNumber);
		if (!planeFound) {
			BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] no plane found for format " << DRMHelper::planeformatString(planeFormat);
			goto cleanup;
		}
	}

	err = drmModeSetPlane(drmFd, planeId, connectionData.crtcId,
		framebufferId, 0,
		this->compositionGeometry.x,
		this->compositionGeometry.y,
		this->compositionGeometry.width,
		this->compositionGeometry.height,
		0, 0,
		connectionData.connectorWidth << 16,
		connectionData.connectorHeight << 16);
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] couldn't connect framebuffer to plane: " << strerror(errno);
		goto cleanup;
	}

	DRMHelper::setPixelBlendCoverageForPlane(planeId);

cleanup:

	return;

}

DRMFrameBuffer::~DRMFrameBuffer()
{
}

int DRMFrameBuffer::getHeight() const
{
	return height;
}

int DRMFrameBuffer::getWidth() const
{
	return width;
}

uint8_t* DRMFrameBuffer::getBuffer() const
{
	return framebufferData;
}

const int DRMFrameBuffer::getBufferLen() const
{
	return framebufferLen;
}

void DRMFrameBuffer::addImage(RGBBuffer& image, int x, int y)
{
	int max_x = x + image.width;
	int max_y = y + image.height;
	int start_x = (x >= 0) ? 0 : -x;
	int start_y = (y >= 0) ? 0 : -y;

	int px_len = 4;

	if (max_x > width) { max_x = width; };
	if (max_y > height) { max_y = height; };

	for (int src_y = start_y, dst_y = y; src_y < max_y; src_y++, dst_y++) {
		uint8_t* dst = framebufferData + (dst_y * width + x)*4; // destination pixel address
		auto start = image.getData()[0];
		uint8_t* src = image.getDataPointer() + (src_y * image.width*px_len); // source pixel
		for (int src_x = start_x; src_x < max_x; src_x++) {
			uint8_t test[4] = { 0xff,0,0,0x7f };
			memcpy(dst, test, px_len);
			dst += 4;
			src += px_len;
		}
		y++;
	}

	return;
}
