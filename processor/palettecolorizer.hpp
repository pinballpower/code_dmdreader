#pragma once

#include <vector>

#include "../dmd/color.hpp"
#include "frameprocessor.hpp"

class PaletteColorizer : public DMDFrameProcessor {

public:
	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:

	vector<DMDColor> palette;
};