#include "drmtest.hpp"

#include <vector>
#include "../drm/drmhelper.hpp"
#include "../drm/drmframebuffer.hpp"

void createDummyImage(uint8_t* framebufferData, int framebufferLen, int offset) {
	int j = offset;
	for (int i = 0; i < framebufferLen; i += 4, j++) {
		framebufferData[i] = j & 0xff;
		framebufferData[i + 1] = (j / 8) & 0xff;
		framebufferData[i + 2] = (j / 256) & 0xff;
		framebufferData[i + 3] = j & 0xff;
	}
}


vector<DRMFrameBuffer> fb;

void demo_framebuffer() {
	DRMHelper dh = DRMHelper();
	dh.initFullscreen(0);
	for (int i = 6; i < 8; i++) {
		DRMFrameBuffer x = DRMFrameBuffer(0, i, CompositionGeometry());
		createDummyImage(x.getBuffer(), x.getBufferLen(), i*7);
		fb.push_back(std::move(x));
	}
}