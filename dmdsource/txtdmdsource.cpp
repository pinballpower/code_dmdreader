#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <filesystem>

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
	openFile(filename);
}

TXTDMDSource::~TXTDMDSource()
{
}

bool TXTDMDSource::openFile(string filename)
{
	is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		is.open(filename);
	}
	catch (std::ios_base::failure e) {
		BOOST_LOG_TRIVIAL(error) << "[txtdmdsource] can't open file " << filename << ": " << e.what();
		eof = true;
		return false;
	}

	BOOST_LOG_TRIVIAL(info) << "[txtdmdsource] successfully opened " << filename;
	return true;
}

void TXTDMDSource::preloadNextFrame()
{
	try {
		// Look for checksum
		bool timestamp_found = false;
		string line;
		regex timestamp_regex("(0x|\\$)[0-9a-fA-F]{8}.*");
		while (!timestamp_found) {
			if (!std::getline(is, line)) {
				if (std::cin.eof()) {
					eof = true;
				}
				else {
					throw std::ios_base::failure("couldn't read file");
				}
			}
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
			if (!std::getline(is, line)) {
				if (std::cin.eof()) {
					eof = true;
				}
				else {
					throw std::ios_base::failure("couldn't read file");
				}
			}
			rtrim(line);
			len = line.length();
			if (len > width) {
				width = len;
			}
			frametxt.push_back(line);
		}

		int height = frametxt.size() - 1;

		// Initialize frame
		preloadedFrame = DMDFrame(width, height, bits);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				uint8_t pv = frametxt[y][x] - '0';
				preloadedFrame.appendPixel(pv);
			}
		}
	}
	catch (std::ios_base::failure e) {
		if (!is.eof()) {
			BOOST_LOG_TRIVIAL(error) << "[txtdmdsource] error reading file " << e.what();
		}

		eof = true;
	}
}

DMDFrame TXTDMDSource::getNextFrame()
{
	DMDFrame res = std::move(preloadedFrame);
	preloadNextFrame();
	return res;
}

bool TXTDMDSource::isFinished()
{
	return eof;
}

bool TXTDMDSource::isFrameReady()
{
	return (!eof);
}


SourceProperties TXTDMDSource::getProperties() {
	return SourceProperties(preloadedFrame);
}

bool TXTDMDSource::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	bits = pt_source.get("bitsperpixel", 4);
	bool res=openFile(pt_source.get("name", ""));
	if (res) preloadNextFrame();
	return res;
}