#include <string>

#include <boost/log/trivial.hpp>

#include "pngsource.hpp"
#include "../util/image.hpp"

DMDFrame PNGSource::getNextFrame()
{
	DMDFrame frame = frames.front();
	frames.pop();
	return frame;
}

bool PNGSource::isFinished()
{
	return frames.empty();
}

bool PNGSource::isFrameReady()
{
	return true;
}

bool PNGSource::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	for (auto pair : pt_source.get_child("files"))
	{
		BOOST_LOG_TRIVIAL(info) << "[pngsource] reading " << pair.second.data();
		RGBBuffer buff = RGBBuffer::fromImageFile(pair.second.data());
		if (!buff.isNull()) {
			frames.push(DMDFrame(buff));
		}
	}
	return true;
}
