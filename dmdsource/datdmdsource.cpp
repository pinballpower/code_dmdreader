#include <boost/log/trivial.hpp>

#include "datdmdsource.h"

DATDMDSource::DATDMDSource()
{
}

DATDMDSource::DATDMDSource(string filename)
{
	read_file(filename);
}

DATDMDSource::~DATDMDSource()
{
	// TODO: clear
}

bool DATDMDSource::read_file(string filename)
{
	ifstream df(filename.c_str(), ios::in | ios::binary);
	if (!df) {
		BOOST_LOG_TRIVIAL(error) << "can't open file " << filename;
		return false;
	}

	df.exceptions(ifstream::failbit | ifstream::badbit);

	int framecount = 0;
	try
	{
		int rc = 0;
		while (rc == 0) {
			DMDFrame* frame = this->read_from_dat(df);
			if (frame) {
				frames.push(frame);
				framecount++;
			}
		}
	}
	catch (int e) {
		BOOST_LOG_TRIVIAL(error) << "I/O error reading " << filename << ": " << e;
		return false;
	}

	BOOST_LOG_TRIVIAL(info) << "successfully loaded " << framecount << " from " << filename;

	return true;
}

DMDFrame* DATDMDSource::read_from_dat(std::ifstream& fis)
{
	if ((!fis.good()) || fis.eof()) {
		return nullptr;
	}

	uint8_t header[8];
	DMDFrame* res = nullptr;

	try {
		fis.read((char*)header, 8);
		int rows = (header[0] << 8) + header[1];
		int columns = (header[2] << 8) + header[3];
		int bitsperpixel = (header[6] << 8) + header[7];

		assert((bitsperpixel == 1) || (bitsperpixel == 2) || (bitsperpixel == 4) || (bitsperpixel == 8));
		int datalen = rows * columns * bitsperpixel / 8;
		int px_per_byte = 8 / bitsperpixel;

		res = new DMDFrame(columns, rows, bitsperpixel);

		char* buf = new char[rows * columns * bitsperpixel / 8];
		uint8_t* current_px = (uint8_t*)buf;
		fis.read(buf, datalen);
		// data are packed, convert these to plain bytes
		for (int i = 0; i < datalen; i++, current_px++) {
			uint8_t px = *current_px;
			for (int bit = 8 - bitsperpixel; bit >= 0; bit -= bitsperpixel) {
				uint8_t pv = (px >> bit) & res->get_pixelmask();
				res->add_pixel(pv);
			}
		}
		delete[] buf;
	}
	catch (std::ios_base::failure) {
		return nullptr;
	}

	return res;
}


unique_ptr<DMDFrame> DATDMDSource::next_frame(bool blocking)
{
	DMDFrame* res = frames.front();
	frames.pop();
	return std::unique_ptr<DMDFrame>(res);
}

bool DATDMDSource::finished()
{
	return frames.empty();
}

bool DATDMDSource::frame_ready()
{
	return (!finished());
}


void DATDMDSource::get_properties(SourceProperties* p) {
	DMDFrame* frame = frames.front();
	if (frame) {
		p->width = frame->get_width();
		p->height = frame->get_height();
		p->bitsperpixel = frame->get_bitsperpixel();
	}
	else {
		p->width = p->height = p->bitsperpixel = 0;
	}

}

bool DATDMDSource::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	return read_file(pt_source.get("name", ""));
}