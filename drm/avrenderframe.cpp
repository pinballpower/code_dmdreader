#include "avrenderframe.hpp"

AVRenderFrame::AVRenderFrame(AVFrame* frame)
{
	this->frame = frame;
	frameInfo.width = av_frame_cropped_width(frame);
	frameInfo.height = av_frame_cropped_height(frame);

	const AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)frame->data[0];
	int n = 0;
	for (int i = 0; i < desc->nb_layers; i++) {
		for (int j = 0; (j < desc->layers[i].nb_planes) && (j<4); j++) {
			const AVDRMPlaneDescriptor* const p = desc->layers[i].planes + j;
			const AVDRMObjectDescriptor* const obj = desc->objects + p->object_index;
			frameInfo.pitches[n] = p->pitch;
			frameInfo.offsets[n] = p->offset;
			frameInfo.modifiers[n] = obj->format_modifier;
			n++;
		}
	}
}

AVRenderFrame::~AVRenderFrame()
{
	av_frame_free(&frame);
}

uint32_t AVRenderFrame::getFourCC() const
{
	if (frame) {
		const AVDRMFrameDescriptor* desc = (AVDRMFrameDescriptor*)frame->data[0];
		return desc->layers[0].format;
	}
	else {
		return 0;
	}
}

const FrameInfo AVRenderFrame::getFrameInfo() const
{
	return frameInfo;
}
