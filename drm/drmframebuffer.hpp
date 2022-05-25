#pragma once

#include "drmhelper.hpp"
#include "../util/image.hpp"

class DRMFrameBuffer {

public:
	DRMFrameBuffer(int screenNumber, int planeNumber, const CompositionGeometry geometry);
	~DRMFrameBuffer();

	int getWidth() const;
	int getHeight() const;
	uint8_t* getBuffer() const;
	const int getBufferLen() const;

	void addImage(RGBBuffer& image, int x, int y);

private:
	int screenNumber = 0;
	int planeNumber = 0;
	uint32_t planeId = 0;

	int drmFd = 0;

	DRMConnectionData connectionData;
	CompositionGeometry compositionGeometry;

	uint8_t* framebufferData = nullptr;
	int framebufferLen = 0;

	int width = 0;
	int height = 0;
	int bytesPerLine = 0;

};
