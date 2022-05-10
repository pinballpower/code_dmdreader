#include <iomanip>

#include <boost/log/trivial.hpp>

#include "framestore.hpp"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


FrameStore::~FrameStore()
{
}



bool FrameStore::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	string name = pt_source.get("filename", "");
	if (name == "") {
		BOOST_LOG_TRIVIAL(error) << "filename has not been configured, aborting";
		return false;
	}

	ignore_duplicates = pt_source.get("ignore_duplicates", true);
	async = pt_source.get("async", true);

	outputfile.open(name, ios::out);
	isFinished = false;

	startMillisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	return true;

}


DMDFrame FrameStore::processFrame(DMDFrame& f)
{
	if (f.getBitsPerPixel() > 8) {
		BOOST_LOG_TRIVIAL(debug) << "[framestore] storing colored frames not supported";
		return f;
	}

	if (!ignore_duplicates) {
		uint32_t checksum = f.getChecksum();
		if (seen.contains(checksum)) {
			BOOST_LOG_TRIVIAL(trace) << "[framestore] frame seen before, ignoring";
			return f;
		}
		seen.insert(checksum);
	}

	if (!isFinished) {
		unsigned long nowMillisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		uint32_t timestamp = nowMillisec - startMillisec;

		if (async) {
			frames_to_write.push(make_pair(f, timestamp));
		}
		else {
			writeFrameToFile(f, timestamp);
		}
	}
	return f;
}

void FrameStore::close()
{
	if (!frames_to_write.empty()) {
		BOOST_LOG_TRIVIAL(info) << "[framestore] writing " << frames_to_write.size() << " frames";
	}
	while (!frames_to_write.empty()) {
		auto nextFrame = frames_to_write.front();
		writeFrameToFile(nextFrame.first, nextFrame.second);
		frames_to_write.pop();
	}
	outputfile.close();
	BOOST_LOG_TRIVIAL(debug) << "[framestore] " << frameno << " frames written";
}

void FrameStore::writeFrameToFile(DMDFrame& f, uint32_t timestamp)
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

		BOOST_LOG_TRIVIAL(trace) << "[framestore] frame " << frameno << " written";

		frameno++;
	}
	else {
		BOOST_LOG_TRIVIAL(debug) << "[framestore] outut file closed, doing nothing";
	}
}
