#pragma once

#include "frameprocessor.hpp"

class SerumColorizer : public DMDFrameProcessor {

public:
	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:
	int width, height;

	bool colorizeUndetectedFrames = false;
};

