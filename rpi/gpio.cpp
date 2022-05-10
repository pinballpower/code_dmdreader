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

void GPIO::setupGPIO(int gpiono, bool output, GPIOEdge edge) {

	string gpios = std::to_string(gpiono);

	// Export GPIO

	try {
		ofstream exportfile;
		exportfile.exceptions(ifstream::badbit | ifstream::failbit);
		exportfile.open("/sys/class/gpio/export");
		exportfile << gpiono;
	}
	catch (std::ifstream::failure e) {
		BOOST_LOG_TRIVIAL(error) << "[gpio] " << e.what();
		throw GPIOException("error writing /sys/class/gpio/export", gpiono);
	}

	// Set direction
	string dirname = "/sys/class/gpio/gpio" + gpios + "/direction";
	ofstream directionfile;
	directionfile.exceptions(ifstream::badbit | ifstream::failbit);
	try {
		directionfile.open(dirname);
		if (output) {
			directionfile << "out\n";
		}
		else {
			directionfile << "in\n";
		}
	}
	catch (std::ifstream::failure e) {
		throw GPIOException("error writing " + dirname, gpiono);
	}

	// Set edge detection
	string edge_detect = GPIO::asEdgeStr(edge);
	if (edge_detect != "") {

		string edgename = "/sys/class/gpio/gpio" + gpios + "/edge";
		ofstream edgefile;
		edgefile.exceptions(ifstream::badbit | ifstream::failbit);
		edgefile.open(edgename);
		try {
			edgefile << edge_detect << "\n";
		}
		catch (std::ifstream::failure e) {
			throw GPIOException("error writing " + edgename, gpiono);
		}
	}
}

int GPIO::getValueFd(int gpiono) {
	if (value_fd.count(gpiono) > 0) {
		return value_fd.at(gpiono);
	}
	else {
		string dirname = "/sys/class/gpio/gpio" + std::to_string(gpiono) + "/value";
		int fd = open(dirname.c_str(), O_RDONLY);
		if (fd <= 0) {
			throw GPIOException("unable to open " + dirname, gpiono);
		}
		value_fd.insert({ gpiono, fd });
		return fd;
	}
}

GPIO::GPIO() {

}

GPIO::~GPIO()
{
	// close open files
	for (auto it : value_fd) {
		close(it.second);
	}
}


bool GPIO::getValue(int gpiono)
{
	int fd = getValueFd(gpiono);

	if (fd <= 0) {
		BOOST_LOG_TRIVIAL(error) << "[gpio] error reading GPIO " << gpiono << " value";
		throw GPIOException("error opening GPIO value file", gpiono);
	}

	char val;
	lseek(fd, 0, SEEK_SET);
	read(fd, &val, 1);
	if (val == '1') {
		return true;
	}
	else if (val == '0') {
		return false;
	}
	else {
		throw GPIOException("value file contains character other than 0 or 1", gpiono);
	}

	return false;
}

bool GPIO::waitForEdge(int gpiono, int timeout)
{
	assert(timeout != 0);

	int fd = getValueFd(gpiono);

	if (fd <= 0) {
		throw GPIOException("error opening GPIO value file", gpiono);
	}

	pollfd pfd = { fd, POLLPRI, 0 };
	poll(&pfd, 1, timeout);

	if (pfd.revents & POLLPRI) {
		// dummy read
		const int readlen = 128;
		char* buf[readlen];
		lseek(fd, 0, SEEK_SET);
		read(fd, buf, readlen);
		return true;
	}
	else {
		return false;
	}

}
