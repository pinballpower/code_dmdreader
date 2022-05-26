#pragma once

#include <queue>
#include <string>
#include <thread>

#include "service.hpp"

using namespace std;

class RecordedEvent {

public:
	RecordedEvent(int timestamp, string event);

	int timestamp;
	string event;
};


class PUPEventPlayback : public Service {

public:
	PUPEventPlayback();
	~PUPEventPlayback();

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;
	virtual bool start() override;
	virtual void stop() override;
	virtual string name() const override;

private:
	void playEvents();
	queue<RecordedEvent> eventsToPlay;

	thread eventPlayerThread;
};