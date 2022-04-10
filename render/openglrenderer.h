#pragma once
#include "../dmd/color.h"
#include "../dmd/palette.h"

#include "raylib.h"
#include "framerenderer.h"

class OpenGLRenderer : public FrameRenderer
{
public:

	
	OpenGLRenderer();
	~OpenGLbRenderer();
	virtual void render_frame(DMDFrame& f);
	void set_palette(const DMDPalette p);

	void set_display_parameters(int width, int height, int px_radius, int px_spacing, int bitsperpixel);
	void start_display();
	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer);

private:

	int width = 0;
	int height = 0;
	int px_radius = 0;
	int px_spacing = 0;

	DMDPalette palette;
};