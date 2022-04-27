#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "frameinfologger.h"

FrameInfoLogger::FrameInfoLogger()
{
}

FrameInfoLogger::~FrameInfoLogger()
{
}

bool FrameInfoLogger::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	return true;
}


DMDFrame FrameInfoLogger::processFrame(DMDFrame &f)
{
	BOOST_LOG_TRIVIAL(info) << "[frameinfologger] got frame " << f.get_width() << "x" << f.get_height() << " " << f.get_bitsperpixel() << "bpp, checksum " << f.get_checksum();
	return f;
}
