#pragma once

#include <boost/log/trivial.hpp>

/*
* A simple object factory. As the number of classes is limited, all mappings are hardcoded.
*/

#include "../dmdsource/dmdsource.h"
#include "../processor/frameprocessor.h"
#include "../render/framerenderer.h"

DMDSource* createSource(string name);
DMDFrameProcessor* createProcessor(string name);
FrameRenderer* createRenderer(string name);