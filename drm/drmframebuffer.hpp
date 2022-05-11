#pragma once

#include "drmhelper.hpp"

class DRMFrameBuffer {

public:
	DRMFrameBuffer(int screenNumber, int planeNumber, const CompositionGeometry geometry);
	~DRMFrameBuffer();

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

};