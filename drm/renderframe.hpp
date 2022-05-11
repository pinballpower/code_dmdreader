#pragma once

#include <cstdint>

struct FrameInfo {
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t pitches[4] = { 0 };
	uint32_t offsets[4] = { 0 };
	uint64_t modifiers[4] = { 0 };
};

class RenderFrame {

public: 
	virtual uint32_t getFourCC() const;
	virtual const FrameInfo getFrameInfo() const;


};