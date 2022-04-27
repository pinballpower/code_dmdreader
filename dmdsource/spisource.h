#pragma once

#include <queue>
#include <thread>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include "dmdsource.h"

#include "../rpi/gpio.h"
#include "../rpi/spi.h"

using namespace std;

constexpr uint16_t FRAME_ID = 0xcc33;
constexpr int MAX_QUEUED_FRAMES = 100;

class SPISource : public DMDSource {
public:

	SPISource();
	~SPISource();

	virtual DMDFrame getNextFrame() override;

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;
	virtual int getDroppedFrames() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual SourceProperties getProperties() override;

private:

	GPIO gpio;
	SPI spi;
	int notify_gpio = 0;
	int droppedFrames = 0; 

	void loopSPIRead();

	queue<DMDFrame> queuedFrames;
	boost::interprocess::interprocess_semaphore frameSemaphore = boost::interprocess::interprocess_semaphore(0);
	thread pollerThread;
	bool active = false;
};