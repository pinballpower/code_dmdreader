#include <string>

#include <boost/log/trivial.hpp>

#include "pngsource.hpp"
#include "../util/image.hpp"

DMDFrame PNGSource::getNextFrame()
{
	if (frames.empty()) {
		return lastFrame;
	}

	DMDFrame frame = frames.front();
	lastFrame=frame;
	frames.pop();
	return frame;
}

bool PNGSource::isFinished()
{
	if (no_finish) {
		return false;
	} else {
		return frames.empty();
	}
}

bool PNGSource::isFrameReady()
{
	return (! frames.empty());
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

        no_finish = pt_source.get("no_finish", true);

	return true;
}
