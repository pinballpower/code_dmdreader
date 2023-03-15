#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <thread>

#include <boost/log/trivial.hpp>

#include "txtdmdsource.hpp"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


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

uint32_t TXTDMDSource::getCurrentTimestamp()
{
	unsigned long now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	return now - startMillisec;
}

void TXTDMDSource::preloadNextFrame()
{
	try {
		// Look for checksum
		bool timestamp_found = false;
		string line;
		regex timestamp_regex("(0x|\\$)([0-9a-fA-F]{8}).*");
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

			smatch matches;
			if (regex_search(line, matches, timestamp_regex)) {
				// parse timestamp to uint32_t
				timestamp_found = true;
				stringstream ss;
				ss << std::hex << matches[2].str();
				ss >> preloadedFrameTimestamp;
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
		id++;
		preloadedFrame.setId(id);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				uint8_t pv = frametxt[y][x];
				if (pv <= '9') {
					pv = pv - '0';
				} else {
					pv = pv - 'a'+10;
				}
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
	DMDFrame res;
	uint32_t timestamp = getCurrentTimestamp();

	if ((useTimingData) && (timestamp < preloadedFrameTimestamp)) {

		// no new frame from TXT file yet ready

		if (frameEveryMs > 0) {
			uint32_t sleepms = frameEveryMs - (timestamp - lastFrameSentMillis);
			this_thread::sleep_for(std::chrono::milliseconds(sleepms));
		}

		timestamp = getCurrentTimestamp();

		if (timestamp < preloadedFrameTimestamp) {
			res = DMDFrame(currentFrame);
		}
		else {
			while (timestamp < preloadedFrameTimestamp) {
				this_thread::sleep_for(std::chrono::milliseconds(10));
				timestamp = getCurrentTimestamp();
			}
			currentFrame = preloadedFrame;
			res = std::move(preloadedFrame);
			preloadNextFrame();

		}

	} else {
		currentFrame = preloadedFrame;
		res = std::move(preloadedFrame);
		preloadNextFrame();
	}

	lastFrameSentMillis = getCurrentTimestamp();
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
	useTimingData = pt_source.get("use_timing_data", true);
	frameEveryMs = pt_source.get("frame_every_ms", 0);
	bool res=openFile(pt_source.get("name", ""));
	if (res) preloadNextFrame();
	startMillisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	return res;
}