#pragma once

#include "frameprocessor.hpp"

enum class Upscaler { hq2x, hq3x, hq4x };

class Upscale : public DMDFrameProcessor {

public:
	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:
	Upscaler upscaler;
};
