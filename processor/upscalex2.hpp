#pragma once

#include "frameprocessor.hpp"

class UpscaleX2 : public DMDFrameProcessor {

public:
	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:
	int width, height;
};

