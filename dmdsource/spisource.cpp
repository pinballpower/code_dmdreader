#include <boost/log/trivial.hpp>

#include "spisource.h"
#include "../rpi/spi.h"

uint16_t parse_u16(uint8_t* buf) {
	return buf[1] + 256 * buf[0];
}

/// <summary>
/// A loop that simply reads endlessly from SPI
/// </summary>
void SPISource::loopSPIRead() {
	int length, packet_type;
	int rows, columns, bitsperpixel;

	int frames = 0;

	uint8_t buf[10000];


	while (true) {

		bool edge_detected = true;
		bool x = false;
		if (!gpio.getValue(notify_gpio)) {
			BOOST_LOG_TRIVIAL(error) << "*";
			x = true;
			edge_detected = gpio.waitForEdge(notify_gpio, 500);
		}

		// If there is no edge detected, just do some dummy reads, we might just be within a frame
		if (!edge_detected) {
			BOOST_LOG_TRIVIAL(error) << "[spisource] dummy SPI read as no edge was detected";
			spi.readData(buf, 256);
			continue;
		}

		// Read 4 byte header
		try {
			spi.readData(buf, 4);
		}
		catch (std::ios::failure e) {
			BOOST_LOG_TRIVIAL(error) << "[spisource] IO error " << e.what();
			continue;
		}

		length = parse_u16(buf);
		packet_type = parse_u16(buf + 2);

		if (x) {
			bool y = true;
		}

		if ((packet_type == 0xcc33) || (packet_type == 0xcc33)) {
			bool y = true;
			x = true;
		}
		else {
			x = false;
		}

		// Check if the header is valid
		if (length < 4) {
			BOOST_LOG_TRIVIAL(trace) << "[spisource] received invalid packet length " << length << ", ignoring";
			continue;
		}

		if (packet_type == 0) {
			continue;
		}

  		spi.readData(buf, length - 4);

		if (x) {
			bool y = false;
		}

		if (packet_type == 0xcc33) {
			rows = parse_u16(buf + 0);
			columns = parse_u16(buf + 2);
			bitsperpixel = parse_u16(buf + 6);

			if (((columns != 128) && (columns != 192)) ||
				((rows != 32) && (rows != 64)) ||
				((bitsperpixel != 2) && (bitsperpixel != 4))) {
				BOOST_LOG_TRIVIAL(trace) << "[spisource] resolution " << columns << "x" << rows << "x" << bitsperpixel << " unsupported, ignoring";
				continue;
			}
			else {
				BOOST_LOG_TRIVIAL(info) << "[spisource] got frame " << columns << "x" << rows << "x" << bitsperpixel;
			}

			// TODO: create frame
		}
		else {
			BOOST_LOG_TRIVIAL(debug) << "[spisource] packet type " << packet_type << "unsupported, ignoring";
		}

		frames++;

	}
}



SPISource::SPISource()
{
}

DMDFrame SPISource::getNextFrame(bool blocking)
{
	return DMDFrame();
}

bool SPISource::isFinished()
{
	return false;
}

bool SPISource::isFrameReady()
{
	return false;
}

bool SPISource::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string device_name = pt_source.get("device", "/dev/spidev1.0");
	unsigned int spi_speed = pt_source.get("speed", 1000000);
	notify_gpio = pt_source.get("notify_gpio", 7);

	//
	// Setup SPI
	//
	try {
		spi.openDevice(device_name, spi_speed);
	}
	catch (SPIException e) {
		BOOST_LOG_TRIVIAL(error) << "[spisource] failed to initialize SPI source: " << e.msg;
		return false;
	}
	BOOST_LOG_TRIVIAL(info) << "[spisource] connect to SPI via " << device_name << ", speed=" << spi_speed;

	//
	// Setup GPIO
	//
	try {
		gpio.setupGPIO(notify_gpio, false, GPIOEdge::raising);
	}
	catch (GPIOException e) {
		BOOST_LOG_TRIVIAL(info) << "[spisource] failed to configure edge detection on " << notify_gpio << ": " << e.msg;
		return false;
	}

	loopSPIRead();

	return true;
}

SourceProperties SPISource::getProperties()
{
	return SourceProperties();
}

