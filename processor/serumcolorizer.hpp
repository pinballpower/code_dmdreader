#pragma once

#include "frameprocessor.hpp"

#include <cstdint>
#include "../external/libserum/src/serum-decode.h"
#include "../external/libserum/src/serum-version.h"

#define SERUM_MAXWIDTH 192
#define SERUM_MAXHEIGHT 64

class SerumColorizer : public DMDFrameProcessor {

public:
	SerumColorizer();

	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:
	int width, height;

	bool colorizeUndetectedFrames = false;
	uint32_t checksumLastFrame = 0;

	uint32_t timestampLastFrame = 0;

	uint8_t srcbuffer[SERUM_MAXWIDTH * SERUM_MAXHEIGHT];
	uint8_t palette[PALETTE_SIZE];
	uint8_t rotations[ROTATION_SIZE];
};

