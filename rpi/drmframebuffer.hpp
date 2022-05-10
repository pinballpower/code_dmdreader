#pragma once

#include "drmhelper.hpp"

class DRMFrameBuffer {

public:
	DRMFrameBuffer(int screenNumber, int planeNumber);
	~DRMFrameBuffer();

private:
	int screenNumber = 0;
	int planeNumber = 0;

	int drmFd = 0;

	DRMConnectionData connectionData;

};