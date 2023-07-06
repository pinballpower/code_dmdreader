#pragma once

#include <vector>

#include "../util/data.hpp"
#include "../dmd/color.hpp"
#include "frameprocessor.hpp"

class PaletteColorizer : public DMDFrameProcessor {

public:

	PaletteColorizer();

	virtual DMDFrame processFrame(DMDFrame& f) override;
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:

	vector<DMDColor> palette;
};


DMDFrame highlightRectangles(const DMDFrame& f, const DMDPalette& palette, const vector<Rectangle>& highlightRectangles);