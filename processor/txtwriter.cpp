#include <iomanip>

#include <boost/log/trivial.hpp>

#include "txtwriter.hpp"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


TXTWriter::~TXTWriter()
{
	close();
}



bool TXTWriter::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string name = pt_source.get("filename", "");
	if (name == "") {
		BOOST_LOG_TRIVIAL(error) << "filename has not been configured, aborting";
		return false;
	}
	outputfile.open(name, ios::out);

	return GenericWriter::configureFromPtree(pt_general, pt_source);
}

void TXTWriter::close()
{
	if (!frames_to_write.empty()) {
		BOOST_LOG_TRIVIAL(info) << "[TXTWriter] writing " << frames_to_write.size() << " frames";
	}
	while (!frames_to_write.empty()) {
		auto nextFrame = frames_to_write.front();
		writeFrameToFile(nextFrame.first, nextFrame.second);
		frames_to_write.pop();
	}
	outputfile.close();
	BOOST_LOG_TRIVIAL(debug) << "[TXTWriter] " << frameno << " frames written";
}

void TXTWriter::writeFrameToFile(DMDFrame& f, uint32_t timestamp)
{
	if (outputfile.is_open()) {
		outputfile << "$" << std::hex << std::setw(8) << std::setfill('0') << timestamp << std::endl;

		string line = "";
		int col = 0;
		for (auto px : f.getPixelData()) {
			line.push_back('0' + (char)px);

			if (col == f.getWidth() - 1) {
				outputfile << line << std::endl;
				line = "";
				col = 0;
			}
			else {
				col++;
			}
		}

		assert(col == 0); // there should be no pixel left

		outputfile << std::endl;

		BOOST_LOG_TRIVIAL(trace) << "[TXTWriter] frame " << frameno << " written";

		frameno++;
	}
	else {
		BOOST_LOG_TRIVIAL(debug) << "[TXTWriter] outut file closed, doing nothing";
	}
}
