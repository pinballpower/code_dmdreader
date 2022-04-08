#include "../dmdsource/dmdsource.h"
#include "../dmdsource/datdmdsource.h"
#include "../dmdsource/txtdmdsource.h"
#include "../dmdsource/nullsource.h"
#include "../dmd/pubcapture.h"
#include "../dmd/frameprocessor.h"
#include "../dmd/frameinfologger.h"
#include "../render/framerenderer.h"
#include "../render/raylibrenderer.h"

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
	else {
		BOOST_LOG_TRIVIAL(error) << "processor name " << name << " unknown";
		return NULL;
	}

}


FrameRenderer* createRenderer(string name) {
	if (name == "raylib") {
		return (FrameRenderer*)(new RaylibRenderer());
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "renderer name " << name << " unknown";
		return NULL;
	}

}
