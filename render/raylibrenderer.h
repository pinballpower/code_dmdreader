#pragma once
#include "../dmd/color.h"
#include "../dmd/palette.h"

#include "raylib.h"
#include "framerenderer.h"

class RaylibRenderer: public FrameRenderer
{
public:

	RaylibRenderer();
	~RaylibRenderer();
	virtual void renderFrame(DMDFrame &f) override;
	void setPalette(const DMDPalette p);

	void setDisplayParameters(int width, int height, int px_radius, int px_spacing, int bitsperpixel);
	void startDisplay();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) override;

private:

	int width = 0;
	int height = 0;
	int px_radius = 0;
	int px_spacing = 0;

	DMDPalette palette;
};