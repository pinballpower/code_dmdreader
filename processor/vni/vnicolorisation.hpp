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
	std::optional<PaletteMapping> findMapForPlaneData(const vector<uint8_t> pd) const;
	void setPalette(const vector<DMDColor> colors);

	// for use as a DMDSource
	int src_current_animation = 0;
	int src_current_frame = 0;
	int src_current_frame_in_animation = 0;

	/// <summary>
	/// Number of all frames in all animations
	/// </summary>
	int src_frame_count;

	//
	// Variables for coloring
	//
	vector<DMDColor> col_palette;
	int col_frames_left = -1;
	Animation col_animation;
	bool animation_active = false;
	int col_anim_frame = 0;
	SwitchMode col_mode = ModePalette;

	// Features defined by the license
	bool allow_vni = false;
};