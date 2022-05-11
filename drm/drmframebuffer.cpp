#include "drmframebuffer.hpp"

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <boost/log/trivial.hpp>

#define NAME_ALPHA "alpha"
#define NAME_PIXEL_BLEND "pixel blend mode"
#define VALUE_PIXEL_BLEND "Coverage"

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


void createDummyImage(uint8_t* framebufferData, int framebufferLen, int offset) {
	int j = offset;
	for (int i = 0; i < framebufferLen; i += 4, j++) {
		framebufferData[i] = j & 0xff;
		framebufferData[i + 1] = (j / 8) & 0xff;
		framebufferData[i + 2] = (j / 256) & 0xff;
		framebufferData[i + 3] = j & 0xff;
	}
}


void enableAlphaForPlane(int drmFd, uint32_t planeId) {
	drmModeObjectPropertiesPtr props;
	props = drmModeObjectGetProperties(drmFd, planeId,
		DRM_MODE_OBJECT_PLANE);
	if (!props) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] no plane properties found";
	}

	for (int i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop;

		prop = drmModeGetProperty(drmFd, props->props[i]);
		if (prop) {
			if (strcmp(prop->name, NAME_PIXEL_BLEND) == 0) {

				drm_mode_property_enum* pEnum = prop->enums;
				for (int j = 0; j < prop->count_enums; j++) {
					if (strcmp(pEnum->name, VALUE_PIXEL_BLEND) == 0) {
						BOOST_LOG_TRIVIAL(info) << "[drmframebuffer] setting " << NAME_PIXEL_BLEND << " to " << VALUE_PIXEL_BLEND;
						if (drmModeObjectSetProperty(drmFd, planeId, DRM_MODE_OBJECT_PLANE, prop->prop_id, pEnum->value)) {
							BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not set pixel blending property";
						}
					}
					pEnum++;
				}
			}

			if (strcmp(prop->name, NAME_ALPHA) == 0) {
				BOOST_LOG_TRIVIAL(info) << "[drmframebuffer] setting " << NAME_ALPHA;
				if (drmModeObjectSetProperty(drmFd, planeId, DRM_MODE_OBJECT_PLANE, prop->prop_id, 0xffff)) {
					BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not set alpha property";
				}
			}

			drmModeFreeProperty(prop);
		}
	}
	drmModeFreeObjectProperties(props);
}





DRMFrameBuffer::DRMFrameBuffer(int screenNumber, int planeNumber, const CompositionGeometry geometry)
{
	int err;

	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;
	this->compositionGeometry = geometry;

	drmFd = DRMHelper::getDRMDeviceFd(); // Cache it
	connectionData = DRMHelper::getConnectionData(screenNumber);
	if (! connectionData.connected) {
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not connect to screen " << screenNumber;
	}

	width = connectionData.connectorWidth;
	height = connectionData.connectorHeight;

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
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] could not add framebuffer to drm: " << strerror(errno);
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
		bool planeFound = DRMHelper::findPlane(connectionData.crtcIndex, planeFormat, &planeId, 5);
		if (!planeFound) {
			BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] No plane found for format " << DRMHelper::planeformatString(planeFormat);
			goto cleanup;
		}
	}

	enableAlphaForPlane(drmFd, planeId);

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
		BOOST_LOG_TRIVIAL(error) << "[drmframebuffer] Can't connect framebuffer to plane: " << strerror(errno);
		goto cleanup;
	}

	enableAlphaForPlane(drmFd, planeId);

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