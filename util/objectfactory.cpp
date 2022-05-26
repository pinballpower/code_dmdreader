#include <string>

#include "../dmdsource/dmdsource.hpp"
#include "../dmdsource/datdmdsource.hpp"
#include "../dmdsource/txtdmdsource.hpp"
#include "../dmdsource/nullsource.hpp"
#include "../dmdsource/pngsource.hpp"
#include "../processor/pupcapture.hpp"
#include "../processor/frameprocessor.hpp"
#include "../processor/frameinfologger.hpp"
#include "../processor/palettecolorizer.hpp"
#include "../processor/framestore.hpp"
#include "../render/framerenderer.hpp"
#include "../services/service.hpp"
#include "../services/pupeventplayback.hpp"

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
#include "../pupplayer/pivid.hpp"
#endif

#if __has_include("../colorize/config.h")
#include "../colorize/config.h"
#include "../colorize/pin2dmdcolorisation.hpp"
#endif


std::shared_ptr<DMDSource> createSource(string name) {

	if (name == "dat") {
		return std::make_shared<DMDSource>();
	} else if (name == "txt") {
		return std::make_shared<TXTDMDSource>();
	}
	else if (name == "null") {
		return std::make_shared<NullDMDSource>();
	}
	else if (name == "png") {
		return std::make_shared<PNGSource>();
	}
#ifdef USE_SPI
	else if (name == "spi") {
		return std::make_shared<SPISource>();
	}
#endif
#ifdef VNICOLORING
	else if (name == "vni") {
		return std::make_shared<Pin2DMDColorisation>();
	}
#endif
	else {
		BOOST_LOG_TRIVIAL(error) << "source name " << name << " unknown";
		return NULL;
	}

}

std::shared_ptr<DMDFrameProcessor> createProcessor(string name) {

	if (name == "pupcapture") {
		return std::make_shared<PUPCapture>();
	}
#ifdef VNICOLORING
	else if (name == "pin2dmd") {
		return std::make_shared<Pin2DMDColorisation>();
	}
#endif
	else if (name == "frameinfo") {
		return std::make_shared<FrameInfoLogger>();
	}
	else if (name == "palette") {
		return std::make_shared<PaletteColorizer>();
	}
	else if (name == "store") {
		return std::make_shared<FrameStore>();
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "processor name " << name << " unknown";
		return NULL;
	}

}

std::shared_ptr<FrameRenderer> createRenderer(string name) {
	if (name == "null") {
		return std::make_shared<FrameRenderer>();
	} 
#ifdef USE_OPENGLGLAD
	else if (name == "opengl") {
		return std::make_shared<GladOpenGLRenderer>();
	}
#endif
#ifdef USE_OPENGLPI4
	else if (name == "opengl") {
		return std::make_shared<Pi4Renderer>();
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
		return std::make_shared<PividPUPPlayer>();
	}
#endif
	else if (name == "pupeventplayback") {
		return std::make_shared<PUPEventPlayback>();
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "renderer name " << name << " unknown";
		return NULL;
	}
}
