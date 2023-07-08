#include "ratelimit.hpp"

#include "../../util/counter.hpp"

#define RL_FRAMES				"ratelimite::frames::all"
#define RL_FRAMES_ACCEPTED		"ratelimite::frames::accepted"
#define RL_FRAMES_REJECTED		"ratelimite::frames::rejected"

RateLimit::RateLimit()
{
}

DMDFrame RateLimit::processFrame(DMDFrame& f)
{
	INC_COUNTER(RL_FRAMES);
	if (accept_every > 0) {
		count += 1;
		if (count >= accept_every) {
			count = 0;
			INC_COUNTER(RL_FRAMES_ACCEPTED);
			return f;
		}
		else {
			INC_COUNTER(RL_FRAMES_REJECTED);
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