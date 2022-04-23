#pragma once

#include <vector>

#include "../dmd/color.h"
#include "frameprocessor.h"

class PaletteColorizer : public DMDFrameProcessor {

public:
	virtual DMDFrame process_frame(DMDFrame& f);
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

private:

	vector<DMDColor> palette;
};