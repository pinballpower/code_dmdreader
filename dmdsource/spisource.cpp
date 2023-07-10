#include "spisource.hpp"
#include <thread>

#include <boost/log/trivial.hpp>

#include "spisource.hpp"
#include "../rpi/spi.hpp"
#include "../util/profiler.hpp"

#define PROFILER_FRAME	"spisource::frame"

uint16_t parse_u16(uint8_t* buf) {
	return buf[1] + 256 * buf[0];
}

/// <summary>
/// A loop that simply reads endlessly from SPI
/// </summary>
void SPISource::loopSPIRead() {
	int length, packet_type;
	int rows, columns, bitsperpixel;
        uint32_t crc32 = 0;

	int frames = 0;

	uint8_t buf[10000];


	while (active) {

		bool edge_detected = true;
		bool x = false;
		if (!gpio.getValue(notify_gpio)) {
			x = true;
			edge_detected = gpio.waitForEdge(notify_gpio, max_wait_ms);
		}

		// If there is no edge detected, just do some dummy reads, we might just be within a frame
		if (!edge_detected) {
			BOOST_LOG_TRIVIAL(error) << "[spisource] dummy SPI read as no edge was detected";
			spi.readData(buf, 256);
			continue;
		}

		END_PROFILER(PROFILER_FRAME);
		START_PROFILER(PROFILER_FRAME);

		try {
			// Read 4 byte header
			spi.readData(buf, 4);

			length = parse_u16(buf);
			packet_type = parse_u16(buf + 2);

			// Check if the header is valid
			if (length < 4) {
				BOOST_LOG_TRIVIAL(trace) << "[spisource] received invalid packet length " << length << ", ignoring";
				continue;
			}

			// Read the remainder of the packet
			spi.readData(buf, length - 4);
		}
		catch (std::ios::failure e) {
			BOOST_LOG_TRIVIAL(error) << "[spisource] IO error " << e.what() << ", aborting";
			active = false;
			continue;
		}


		if ((packet_type == FRAME_ID_NOCRC) || (packet_type == FRAME_ID_CRC)) {
                        int data_offset = 8;
			rows = parse_u16(buf + 0);
			columns = parse_u16(buf + 2);
			bitsperpixel = parse_u16(buf + 6);

			if (((columns != 128) && (columns != 192)) ||
				((rows != 32) && (rows != 64)) ||
				((bitsperpixel != 2) && (bitsperpixel != 4))) {
				BOOST_LOG_TRIVIAL(trace) << "[spisource] resolution " << columns << "x" << rows << "x" << bitsperpixel << " unsupported, ignoring";
				continue;
			}

            if (packet_type == FRAME_ID_CRC) {
                    crc32 = 1;
                    data_offset = 12; 
            }

			BOOST_LOG_TRIVIAL(trace) << "[spisource] got frame " << columns << "x" << rows << "x" << bitsperpixel << " CRC=" << crc32;
			int queueLen = queuedFrames.size();
			if (queueLen < MAX_QUEUED_FRAMES) {
				DMDFrame frame = DMDFrame(columns, rows, bitsperpixel, buf + data_offset, true);
				queuedFrames.push(frame);
				frameSemaphore.post();
			} else {
				BOOST_LOG_TRIVIAL(warning) << "[spisource] frame queue is full (" << queueLen << "), dropping frame";
				droppedFrames++;
			}
		}
		else {
			BOOST_LOG_TRIVIAL(info) << "[spisource] packet type " << packet_type << " unsupported, ignoring";
		}

		frames++;

	}
}



SPISource::SPISource()
{
	REGISTER_PROFILER(PROFILER_FRAME, "ms");
}

SPISource::~SPISource()
{
	// gracefully terminate background poller thread
	active = false;
	pollerThread.join();
}

DMDFrame SPISource::getNextFrame()
{
	frameSemaphore.wait();
	DMDFrame frame = queuedFrames.front();
	queuedFrames.pop();
	return frame;
}

bool SPISource::isFinished()
{
	return !active;
}

bool SPISource::isFrameReady()
{
	return !queuedFrames.empty();
}

int SPISource::getDroppedFrames()
{
	return droppedFrames;
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

	return true;
}

void SPISource::start()
{
	active = true;
	pollerThread = std::thread([this] { this->loopSPIRead(); });
}

SourceProperties SPISource::getProperties()
{
	return SourceProperties();
}

