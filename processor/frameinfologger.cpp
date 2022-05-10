#include <iomanip>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "frameinfologger.hpp"

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
	BOOST_LOG_TRIVIAL(info) << "[frameinfologger] got frame " << f.getWidth() << "x" << f.getHeight() << " " << f.getBitsPerPixel() << "bpp, checksum " << std::hex << std::setfill('0') << std::setw(8)  << f.getChecksum();
	return f;
}
