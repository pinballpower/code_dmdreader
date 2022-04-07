#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include <boost/log/trivial.hpp>

#include "txtdmdsource.h"

using namespace std;


TXTDMDSource::TXTDMDSource()
{
}

TXTDMDSource::TXTDMDSource(string filename)
{
	open_file(filename);
}

TXTDMDSource::~TXTDMDSource()
{
	is.close();
}

bool TXTDMDSource::open_file(string filename)
{
	is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	is.open(filename);
	if (!is) {
		BOOST_LOG_TRIVIAL(error) << "[txtdmdsource] can't open file " << filename;
		eof = true;
		return false;
	}

	BOOST_LOG_TRIVIAL(info) << "[txtdmdsource] successfully opened " << filename;

	return true;
}

void TXTDMDSource::read_next_frame()
{
	// Look for checksum
	bool timestamp_found = false;
	char line[255];
	regex timestamp_regex("0x[0-9a-fA-F]{8}");
	while (!timestamp_found) {
		is.getline(line,255);

		if (regex_match(line, timestamp_regex)) {
			timestamp_found = true;
			char* p;
		}
	}

	// read lines
	vector<string> frametxt;
	int width = 0;
	int len = 1;
	while (len) {
		is.getline(line, 255);
		len = strlen(line);
		if (len > width) {
			width = len;
		}
		frametxt.push_back(line);
	}

	int height = frametxt.size()-1;

	// Initialize frame
	preloaded_frame = DMDFrame(width, height, bits);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint8_t pv = frametxt[y][x] - '0';
			preloaded_frame.add_pixel(pv);
		}
	}
}

DMDFrame TXTDMDSource::next_frame(bool blocking)
{
	DMDFrame res = std::move(preloaded_frame);
	try {
		read_next_frame();
	}
	catch (std::ios_base::failure e) {
		if (!is.eof()) {
			BOOST_LOG_TRIVIAL(error) << "[txtdmdsource] error reading file " << e.what();
		}

		eof = true;
		is.close();
	}
	return res;
}

bool TXTDMDSource::finished()
{
	return eof;
}

bool TXTDMDSource::frame_ready()
{
	return (!eof);
}


SourceProperties TXTDMDSource::get_properties() {
	return SourceProperties(preloaded_frame);
}

bool TXTDMDSource::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	bits = pt_source.get("bitsperpixel", 2);
	bool res=open_file(pt_source.get("name", ""));
	if (res) read_next_frame();
	return res;
}