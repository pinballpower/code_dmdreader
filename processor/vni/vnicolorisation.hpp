#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "../../processor/frameprocessor.hpp"
#include "../../dmd/dmdframe.hpp"
#include "../../dmdsource/dmdsource.hpp"
#include "palcoloring.hpp"
#include "vnianimationset.hpp"

class VNIColorisation : public DMDFrameProcessor {

public:

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);
	virtual DMDFrame processFrame(DMDFrame& f) override;

private:

	PalColoring coloring;
	VniAnimationSet animations;

	vector <uint8_t> colorAnimationFrame(const DMDFrame &src_frame, const AnimationFrame &anim_frame, int len);
	std::unique_ptr<PaletteMapping> findMapForPlaneData(const vector<uint8_t> pd) const;
	void setPalette(const vector<DMDColor> colors);
	void setPreviousPalette();
	void setDefaultPalette();
	bool triggerAnimation(const DMDFrame& f);

	/// <summary>
	/// Number of all frames in all animations
	/// </summary>
	int src_frame_count;

	//
	// Variables for coloring
	//
	vector<DMDColor> col_palette;
	vector<DMDColor> previous_col_palette;
	int col_frames_left = -1;
	Animation col_animation;
	SwitchMode col_mode = ModePalette;

	uint32_t crcLastFrame = 0;

	// Features defined by the license
	bool allow_vni = false;
};