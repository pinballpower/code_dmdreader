#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include <boost/log/trivial.hpp>

#include "txtdmdsource.h"

using namespace std;

// trim from end of string (right)
static const char* whitespaces = " \t\n\r\f\v";
inline std::string& rtrim(std::string& s, const char* t = whitespaces)
{
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

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
	try {
		is.open(filename);
	} catch (std::ios_base::failure e) {
		BOOST_LOG_TRIVIAL(error) << "[txtdmdsource] can't open file " << filename << ": " << e.what();
		eof = true;
		return false;
	}

	BOOST_LOG_TRIVIAL(info) << "[txtdmdsource] successfully opened " << filename;
	return true;
}

void TXTDMDSource::preload_next_frame()
{
	try {
		// Look for checksum
		bool timestamp_found = false;
		string line;
		regex timestamp_regex("0x[0-9a-fA-F]{8}.*");
		while (!timestamp_found) {
			std::getline(is, line);
			rtrim(line);

			if (regex_match(line, timestamp_regex)) {
				timestamp_found = true;
			}
		}

		// read lines
		vector<string> frametxt;
		int width = 0;
		int len = 1;
		while (len) {
			std::getline(is, line);
			rtrim(line);
			len = line.length();
			if (len > width) {
				width = len;
			}
			frametxt.push_back(line);
		}

		int height = frametxt.size() - 1;

		// Initialize frame
		preloaded_frame = DMDFrame(width, height, bits);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				uint8_t pv = frametxt[y][x] - '0';
				preloaded_frame.add_pixel(pv);
			}
		}
	}
	catch (std::ios_base::failure e) {
		if (!is.eof()) {
			BOOST_LOG_TRIVIAL(error) << "[txtdmdsource] error reading file " << e.what();
		}

		eof = true;
		is.close();
	}
}

DMDFrame TXTDMDSource::next_frame(bool blocking)
{
	DMDFrame res = std::move(preloaded_frame);
	preload_next_frame();
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
	if (res) preload_next_frame();
	return res;
}