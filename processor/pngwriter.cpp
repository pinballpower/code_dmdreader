#include "pngwriter.hpp"


bool PNGWriter::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	directory = pt_source.get("directory", "output");

	bool ok = colorizer.configureFromPtree(pt_general, pt_source);
	if (!ok) {
		BOOST_LOG_TRIVIAL(warning) << "[PNGWriter] couldn't initialize PaletteColorizer";
	}
	return GenericWriter::configureFromPtree(pt_general, pt_source);
}

PNGWriter::PNGWriter()
{
}

void PNGWriter::writeFrameToFile(DMDFrame& f, uint32_t timestamp)
{
	DMDFrame colored = f;

	// color frame using a standard PaletteColorizer
	if (colored.getBitsPerPixel() <= 8) {
		colored = colorizer.processFrame(colored);
	}

	RGBBuffer rgbBuffer = colored.createRGBBufferFromFrame();
	string fileId = to_string(fileNumber);
	while (fileId.length() < 6) {
		fileId = "0" + fileId;
	}
	string filename = directory + "/frame" + fileId + ".png";
	rgbBuffer.writeToFile(filename);

	fileNumber++;
}
