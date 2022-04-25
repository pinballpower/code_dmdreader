#include <boost/log/trivial.hpp>

#include "spisource.h"
#include "../rpi/spihelper.h"

uint16_t parse_u16(uint8_t* buf) {
    return buf[0] + 256 * buf[1];
}

/// <summary>
/// A loop that simply reads endlessly from SPI
/// </summary>
void spi_read_loop() {
    int length, packet_type;
    int rows, columns, bitsperpixel;

    while (true) {

        // TODO: Wait for GPIO

        // Read 4 byte header
        spi_read(4);
        length = parse_u16(spi_buffer);
        packet_type = parse_u16(spi_buffer+2);

        // Check if the header is valid
        if ((length < 4) || (length > spi_buffer_len)) {
            BOOST_LOG_TRIVIAL(trace) << "[spisource] received invalid packet length " << length << ", ignoring";
            continue;
        }
        if (packet_type > 1) {
            BOOST_LOG_TRIVIAL(trace) << "[spisource] received invalid packet type " << packet_type << ", ignoring";
            continue;
        }

        spi_read(length - 4);

        if (packet_type == 1) {
            columns = parse_u16(spi_buffer + 0);
            rows = parse_u16(spi_buffer + 2);
            bitsperpixel = parse_u16(spi_buffer + 6);

            if (((columns != 128) && (columns != 192)) ||
                ((rows != 32) && (rows != 64)) ||
                ((bitsperpixel != 2) && (bitsperpixel != 4))) { 
                BOOST_LOG_TRIVIAL(trace) << "[spisource] resolution " << rows << "x" << columns << "x" << bitsperpixel << " unsupported, ignoring";
                continue;
            }
            else {
                BOOST_LOG_TRIVIAL(trace) << "[spisource] got frame " << rows << "x" << columns << "x" << bitsperpixel;
            }

            // TODO: create frame
        }
        else {
            BOOST_LOG_TRIVIAL(debug) << "[spisource] packet type " << packet_type << "unsupported, ignoring";
        }
    }
}

/*

*/


SPISource::SPISource()
{
}

DMDFrame SPISource::next_frame(bool blocking)
{
	return DMDFrame();
}

bool SPISource::finished()
{
	return false;
}

bool SPISource::frame_ready()
{
	return false;
}

bool SPISource::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
    string device_name = pt_source.get("device", "/dev/spidev1.0");
    spi_speed = pt_source.get("speed", 10000000);

    if (! spi_open(device_name)) {
        BOOST_LOG_TRIVIAL(error) << "[spisource] failed to initialize SPI source";
        return false;
    }

    BOOST_LOG_TRIVIAL(error) << "[spisource] connect to SPI via " << device_name << ", speed=" << spi_speed;

    spi_read_loop();

	return true;
}

SourceProperties SPISource::get_properties()
{
	return SourceProperties();
}

