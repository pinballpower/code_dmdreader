#include "ratelimit.hpp"

RateLimit::RateLimit()
{
}

DMDFrame RateLimit::processFrame(DMDFrame& f)
{
	if (accept_every > 0) {
		count += 1;
		if (accept_every <= count) {
			return f;
		}
		else {
			return DMDFrame();
		}
	}

	return f;
}

bool RateLimit::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	accept_every = pt_source.get("accept_every", 2);
	return true;
}