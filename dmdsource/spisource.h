#pragma once

#include "dmdsource.h"

#include "../rpi/gpio.h"
#include "../rpi/spi.h"

class SPISource : public DMDSource {
public:

	SPISource();

	virtual DMDFrame getNextFrame(bool blocking = true) override;

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual SourceProperties getProperties() override;

private:

	GPIO gpio;
	SPI spi;
	int notify_gpio = 0;

	void loopSPIRead();

};