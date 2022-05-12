#include "pupplayer.hpp"

#include <queue>

#include <boost/log/trivial.hpp>

#include "../drm/drmhelper.hpp"
#include "../drm/videofile.hpp"
#include "trigger.hpp"

PUPPlayer::~PUPPlayer()
{
}

bool PUPPlayer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	return true;
}

bool PUPPlayer::start()
{
	return true
}

void PUPPlayer::playEvent(int event)
{
}

PUPPlayer::PUPPlayer(int screenNumber)
{
}
