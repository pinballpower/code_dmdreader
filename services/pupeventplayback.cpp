#include "pupeventplayback.hpp"

#include <chrono>
#include <thread>

#include <boost/log/trivial.hpp>
#include <boost/foreach.hpp>

#include "../services/serviceregistry.hpp"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


PUPEventPlayback::PUPEventPlayback()
{
}

PUPEventPlayback::~PUPEventPlayback()
{
}

bool PUPEventPlayback::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt_source.get_child("events")) {
		int ts = v.second.get("timestamp", 0);
		string cmd = v.second.get("event", "");
		eventsToPlay.push(RecordedEvent(ts, cmd));
	}
	BOOST_LOG_TRIVIAL(info) << "[pupeventplayback] loaded " << eventsToPlay.size() << " events";
	return true;
}

bool PUPEventPlayback::start()
{
	eventPlayerThread = thread(&PUPEventPlayback::playEvents, this);
	return true;
}

void PUPEventPlayback::stop()
{
	eventsToPlay = queue<RecordedEvent>(); // empty queue
	if (eventPlayerThread.joinable()) {
		eventPlayerThread.join();
	}
}

string PUPEventPlayback::name() const
{
	return "pupeventplayer";
}

void PUPEventPlayback::playEvents()
{
	unsigned long startMillisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	while (eventsToPlay.size() > 0) {
		auto nextEvent = eventsToPlay.front();
		eventsToPlay.pop();

		unsigned long nowMillisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - startMillisec;
		if (nextEvent.timestamp > nowMillisec) {
			std::this_thread::sleep_for(milliseconds{ nextEvent.timestamp-nowMillisec});
		}
		BOOST_LOG_TRIVIAL(info) << "[pupeventplayback] sending pupcapcture event " << nextEvent.event;

		serviceRegistry.command("pupplayer", nextEvent.event);
	}

	BOOST_LOG_TRIVIAL(info) << "[pupeventplayback] finished sending events";
}

RecordedEvent::RecordedEvent(int timestamp, string event)
{
	this->timestamp = timestamp;
	this->event = event;
}
