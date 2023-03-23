#pragma once

#include <stdint.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "framerenderer.hpp"

class LEDMatrixRenderer : public FrameRenderer
{
public:
	LEDMatrixRenderer();
	~LEDMatrixRenderer();

	virtual void renderFrame(DMDFrame& f);
	virtual void close();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer);

private:
	struct RGBLedMatrix* matrix = NULL;
	struct LedCanvas* offscreen_canvas;

	int width = 128;
	int height = 32;
	bool rotate_180 = false;

};