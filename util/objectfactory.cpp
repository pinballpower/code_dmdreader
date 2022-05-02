#include <string>

#include "../dmdsource/dmdsource.h"
#include "../dmdsource/datdmdsource.h"
#include "../dmdsource/txtdmdsource.h"
#include "../dmdsource/nullsource.h"
#include "../dmdsource/pngsource.h"
#include "../processor/pubcapture.h"
#include "../processor/frameprocessor.h"
#include "../processor/frameinfologger.h"
#include "../processor/palettecolorizer.h"
#include "../processor/framestore.h"
#include "../render/framerenderer.h"
#ifdef USE_RAYLIB
#include "../render/raylibrenderer.h"
#endif
#ifdef USE_OPENGLGLAD
#include "../render/gladopenglrenderer.h"
#endif
#ifdef USE_OPENGLPI4
#include "../render/pi4renderer.h"
#endif
#ifdef USE_SPI
#include "../dmdsource/spisource.h"
#endif

#if __has_include("../colorize/config.h")
#include "../colorize/config.h"
#include "../colorize/pin2dmdcolorisation.h"
#endif


DMDSource* createSource(string name) {

	if (name == "dat") {
		return (DMDSource*)(new DATDMDSource());
	} else if (name == "txt") {
		return (DMDSource*)(new TXTDMDSource());
	}
	else if (name == "null") {
		return (DMDSource*)(new NullDMDSource());
	}
	else if (name == "png") {
		return (DMDSource*)(new PNGSource());
	}
#ifdef USE_SPI
	else if (name == "spi") {
		return (DMDSource*)(new SPISource());
	}
#endif
#ifdef VNICOLORING
	else if (name == "vni") {
		return (DMDSource*)(new Pin2DMDColorisation());
	}
#endif
	else {
		BOOST_LOG_TRIVIAL(error) << "source name " << name << " unknown";
		return NULL;
	}

}

DMDFrameProcessor* createProcessor(string name) {

	if (name == "pubcapture") {
		return (DMDFrameProcessor*)(new PubCapture());
	}
#ifdef VNICOLORING
	else if (name == "pin2dmd") {
		return (DMDFrameProcessor*)(new Pin2DMDColorisation());
	}
#endif
	else if (name == "frameinfo") {
		return (DMDFrameProcessor*)(new FrameInfoLogger());
	}
	else if (name == "palette") {
		return (DMDFrameProcessor*)(new PaletteColorizer());
	}
	else if (name == "store") {
		return (DMDFrameProcessor*)(new FrameStore());
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "processor name " << name << " unknown";
		return NULL;
	}

}


FrameRenderer* createRenderer(string name) {
	if (name == "null") {
		return new FrameRenderer();
	} 
#ifdef USE_RAYLIB
	else if (name == "raylib") {
		return (FrameRenderer*)(new RaylibRenderer());
	}
#endif
#ifdef USE_OPENGLGLAD
	else if (name == "opengl") {
		return (FrameRenderer*)(new GladOpenGLRenderer());
	}
#endif
#ifdef USE_OPENGLPI4
	else if (name == "opengl") {
		return (FrameRenderer*)(new Pi4Renderer());
	}
#endif
	else {
		BOOST_LOG_TRIVIAL(error) << "renderer name " << name << " unknown";
		return NULL;
	}

}
