#pragma once

#include <map>
#include <optional>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "../dmd/maskeddmdframe.hpp"
#include "frameprocessor.hpp"

class PUPCapture : public DMDFrameProcessor {

public:
	PUPCapture();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	bool loadTriggers(int bitsperpixel, string directory, std::optional <DMDPalette> palette);

	virtual DMDFrame processFrame(DMDFrame& f) override;

private:
	DMDPalette palette;
	map<int, MaskedDMDFrame> trigger_frames;
};