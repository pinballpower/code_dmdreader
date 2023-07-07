#include "framerenderer.hpp"

FrameRenderer::FrameRenderer() {
	name = "FrameRenderer";
}

FrameRenderer::~FrameRenderer() {
}

void FrameRenderer::renderFrame(DMDFrame &f) {

}

void FrameRenderer::close()
{
}

bool FrameRenderer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) {
	return false;
}