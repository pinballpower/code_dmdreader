#include <string>

#include "../dmdsource/dmdsource.hpp"
#include "../dmdsource/datdmdsource.hpp"
#include "../dmdsource/txtdmdsource.hpp"
#include "../dmdsource/nullsource.hpp"
#include "../dmdsource/pngsource.hpp"
#include "../processor/pubcapture.hpp"
#include "../processor/frameprocessor.hpp"
#include "../processor/frameinfologger.hpp"
#include "../processor/palettecolorizer.hpp"
#include "../processor/framestore.hpp"
#include "../render/framerenderer.hpp"
#include "../services/service.hpp"

#ifdef USE_OPENGLGLAD
#include "../render/gladopenglrenderer.hpp"
#endif
#ifdef USE_OPENGLPI4
#include "../render/pi4renderer.hpp"
#endif
#ifdef USE_SPI
#include "../dmdsource/spisource.hpp"
#endif
#ifdef USE_VIDEO
#include "../pupplayer/pupplayer.hpp"
#endif

#if __has_include("../colorize/config.h")
#include "../colorize/config.h"
#include "../colorize/pin2dmdcolorisation.hpp"
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


std::shared_ptr<Service> createService(string name) {
	if (name == "null") {
		return std::make_shared<Service>();
	}
#ifdef USE_VIDEO
	else if (name == "pupplayer") {
		return std::make_shared<PUPPlayer>();
	}
#endif
	else {
		BOOST_LOG_TRIVIAL(error) << "renderer name " << name << " unknown";
		return NULL;
	}
}
