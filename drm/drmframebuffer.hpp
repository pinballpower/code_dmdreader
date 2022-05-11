#pragma once

#include "drmhelper.hpp"

class DRMFrameBuffer {

public:
	DRMFrameBuffer(int screenNumber, int planeNumber, const CompositionGeometry geometry);
	~DRMFrameBuffer();

	int getWidth() const;
	int getHeight() const;
	uint8_t* getBuffer() const;
	const int getBufferLen() const;


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

void createDummyImage(uint8_t* framebufferData, int framebufferLen, int offset=0);