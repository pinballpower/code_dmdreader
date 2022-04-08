#pragma once

#include <map>
#include <optional>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "maskeddmdframe.h"
#include "frameprocessor.h"

class PubCapture : DMDFrameProcessor {

public:

	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);
	bool load_triggers(int bitsperpixel, string directory, std::optional <DMDPalette> palette);

	virtual DMDFrame process_frame(DMDFrame& f);

private:
	DMDPalette palette;
	map<int, MaskedDMDFrame> trigger_frames;
};