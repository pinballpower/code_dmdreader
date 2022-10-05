#include <iostream>
#include <fstream>

#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>

#include <boost/log/trivial.hpp>
#include "gpio.hpp"

using namespace std;

GPIOException::GPIOException(const string msg, int gpiono)
{
	this->msg = msg;
	this->gpiono = gpiono;
}

GPIOException::~GPIOException()
{
}

string GPIO::asEdgeStr(GPIOEdge e)
{
	switch (e) {
	case raising:
		return "raising";
	case falling:
		return "falling";
	case both:
		return "both";
	}
	return "";
}

GPIO::GPIO() {
	chip = gpiod::chip("gpiochip0");

}

GPIO::~GPIO()
{
}


bool GPIO::getValue(int gpiono)
{
	auto line = chip.get_line(gpiono);
	line.request({ "GPIO", gpiod::line_request::DIRECTION_INPUT, 0 });
	
	auto val = line.get_value();

	if (val) {
		return true;
	}
	else {
		return false;
	}


}

bool GPIO::waitForEdge(int gpiono, int timeout)
{
	assert(timeout != 0);

	auto line = chip.get_line(gpiono);
	line.request({ "GPIO", gpiod::line_request::EVENT_BOTH_EDGES, 0 }, 1);

	auto events = line.event_wait(::std::chrono::seconds(timeout));
	if (events) {
		return true;
	}
	else {
		return false;
	}

}
