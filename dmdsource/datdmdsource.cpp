#include <boost/log/trivial.hpp>

#include "datdmdsource.h"

DATDMDSource::DATDMDSource()
{
}

DATDMDSource::DATDMDSource(string filename)
{
	readFile(filename);
}

DATDMDSource::~DATDMDSource()
{
}

bool DATDMDSource::readFile(string filename)
{
	ifstream df(filename.c_str(), ios::in | ios::binary);
	if (!df) {
		BOOST_LOG_TRIVIAL(error) << "can't open file " << filename;
		return false;
	}

	df.exceptions(ifstream::failbit | ifstream::badbit);

	while (true) {
		try {
			DMDFrame frame = readFromDatFile(df);
			frames.push(frame);
		} catch (std::ios_base::failure e) {
			break;
		}
	}

	BOOST_LOG_TRIVIAL(info) << "successfully loaded " << frames.size() << " from " << filename;

	return true;
}

DMDFrame DATDMDSource::readFromDatFile(std::ifstream& fis)
{
	DMDFrame res = DMDFrame();
	if ((!fis.good()) || fis.eof()) {
		throw std::ios_base::failure("Error reading from input stream");
	}

	uint8_t header[8];

	fis.read((char*)header, 8);
	int rows = (header[0] << 8) + header[1];
	int columns = (header[2] << 8) + header[3];
	int bitsperpixel = (header[6] << 8) + header[7];

	assert((bitsperpixel == 1) || (bitsperpixel == 2) || (bitsperpixel == 4) || (bitsperpixel == 8));
	int datalen = rows * columns * bitsperpixel / 8;
	int px_per_byte = 8 / bitsperpixel;

	res.setSize(columns, rows, bitsperpixel);

	char* buf = new char[rows * columns * bitsperpixel / 8];
	uint8_t* current_px = (uint8_t*)buf;
	fis.read(buf, datalen);
	// data are packed, convert these to plain bytes
	for (int i = 0; i < datalen; i++, current_px++) {
		uint8_t px = *current_px;
		for (int bit = 8 - bitsperpixel; bit >= 0; bit -= bitsperpixel) {
			uint8_t pv = (px >> bit) & res.getPixelMask();
			res.appendPixel(pv);
		}
	}
	delete[] buf;

	return res;
}


DMDFrame DATDMDSource::getNextFrame(bool blocking)
{
	DMDFrame res = frames.front();
	frames.pop();
	return DMDFrame(res);
}

bool DATDMDSource::isFinished()
{
	return frames.empty();
}

bool DATDMDSource::isFrameReady()
{
	return (!isFinished());
}


SourceProperties DATDMDSource::getProperties() {
	DMDFrame frame = frames.front();
	return SourceProperties(frame.getWidth(), frame.getHeight(), frame.getBitsPerPixel());
}

bool DATDMDSource::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
	return readFile(pt_source.get("name", ""));
}