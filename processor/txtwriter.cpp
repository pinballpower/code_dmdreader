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
		BOOST_LOG_TRIVIAL(error) << "[TXTWriter] filename has not been configured, aborting";
		return false;
	}
	outputfile.open(name, ios::out);

	return GenericWriter::configureFromPtree(pt_general, pt_source);
}

void TXTWriter::close()
{
	GenericWriter::close();
	outputfile.close();
	BOOST_LOG_TRIVIAL(debug) << "[TXTWriter] " << frameno << " frames written";
}

void TXTWriter::writeFrameToFile(DMDFrame& f, uint32_t timestamp)
{
	if (outputfile.is_open()) {
		outputfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << timestamp << std::endl;

		string line = "";
		int col = 0;
		for (auto px : f.getPixelData()) {
                        // this is not a real hexadecimal conversion, but it works well for values from 0-15 that are expected
                        if (px<10) {
			 	line.push_back('0' + (char)px);
                        } else { 
				line.push_back('a' + (char)(px-10));
                        }

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

TXTWriter::TXTWriter() {
	name = "TXTWriter";
}
