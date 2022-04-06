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
	is.open(filename);
	if (!is) {
		BOOST_LOG_TRIVIAL(error) << "can't open file " << filename;
		return false;
	}

	BOOST_LOG_TRIVIAL(info) << "successfully opened " << filename;

	return true;
}

void TXTDMDSource::read_next_frame()
{
	// Look for checksum
	bool checksum_found = false;
	uint32_t crc32 = 0;
	string line;
	regex checksum_regex("0x[0-9a-fA-F]{8}");
	while (!checksum_found) {
		getline(is, line);

		if (regex_match(line, checksum_regex)) {
			checksum_found = true;
			char* p;
			crc32 = strtoul(line.c_str(), &p, 16);
		}
	}

	// read lines
	vector<string> frametxt;
	int width = 0;
	while (line != "") {
		getline(is, line);
		if (line.size() > width) {
			width = line.size();
		}
		frametxt.push_back(line);
	}

	int height = frametxt.size()-1;

	// Initialize frame
	delete frame;
	frame = new DMDFrame(width, height, bits);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint8_t pv = frametxt[y][x] - '0';
			frame->add_pixel(pv);
		}
	}
}

std::unique_ptr<DMDFrame> TXTDMDSource::next_frame(bool blocking)
{
	DMDFrame* res = frame;
	frame = nullptr;
	read_next_frame();
	return std::unique_ptr<DMDFrame>(res);
}

bool TXTDMDSource::finished()
{
	return frame == NULL;
}

bool TXTDMDSource::frame_ready()
{
	return (!finished());
}


void TXTDMDSource::get_properties(SourceProperties* p) {
	if (frame) {
		p->width = frame->get_width();
		p->height = frame->get_height();
		p->bitsperpixel = frame->get_bitsperpixel();
	}
	else {
		p->width = p->height = p->bitsperpixel = 0;
	}

}

bool TXTDMDSource::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	bits = pt_source.get("bitsperpixel", 2);
	bool res=open_file(pt_source.get("name", ""));
	if (res) read_next_frame();
	return res;
}