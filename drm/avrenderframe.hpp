#pragma once

#include "renderframe.hpp"

extern "C" {
#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_drm.h"
#include "libavutil/pixdesc.h"
}

class AVRenderFrame : public RenderFrame {

public:
	AVRenderFrame(AVFrame* frame);
	~AVRenderFrame();

	uint32_t getFourCC() const override;
	virtual const FrameInfo getFrameInfo() const override;


private:

	AVRenderFrame(const  AVRenderFrame&) = delete; // non construction-copyable
	AVRenderFrame& operator=(const  AVRenderFrame&) = delete; // non copyable

	AVFrame* frame;
	FrameInfo frameInfo;

};