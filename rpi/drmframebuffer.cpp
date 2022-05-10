#include "drmframebuffer.hpp"

DRMFrameBuffer::DRMFrameBuffer(int screenNumber, int planeNumber)
{
	this->screenNumber = screenNumber;
	this->planeNumber = planeNumber;
}

DRMFrameBuffer::~DRMFrameBuffer()
{
}
