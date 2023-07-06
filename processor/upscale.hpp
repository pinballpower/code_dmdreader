#pragma once

#include "frameprocessor.hpp"

#include "../external/xbr/filters.h"

enum class Upscaler { hq2x, hq3x, hq4x, xbr2x, xbr3x, xbr4x };

class Upscale : public DMDFrameProcessor {

public:

	Upscale();
	~Upscale();

	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:
	Upscaler upscaler;
	xbr_data* xbrData = NULL;
};
