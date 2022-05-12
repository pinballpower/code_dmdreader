#pragma once

#include <boost/log/trivial.hpp>

/*
* A simple object factory. As the number of classes is limited, all mappings are hardcoded.
*/

#include "../dmdsource/dmdsource.hpp"
#include "../processor/frameprocessor.hpp"
#include "../render/framerenderer.hpp"
#include "../services/service.hpp"

DMDSource* createSource(string name);
DMDFrameProcessor* createProcessor(string name);
FrameRenderer* createRenderer(string name);
std::shared_ptr<Service> createService(string name);