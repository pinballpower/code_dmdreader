#include <iomanip>

#include <boost/log/trivial.hpp>

#include "genericwriter.hpp"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


GenericWriter::GenericWriter()
{
	name = "GenericWriter";
}

GenericWriter::~GenericWriter()
{
	close();
}




bool GenericWriter::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	ignore_duplicates = pt_source.get("ignore_duplicates", true);
	async = pt_source.get("async", true);

	isFinished = false;

	startMillisec = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	return true;

}


DMDFrame GenericWriter::processFrame(DMDFrame& f)
{
	if (! supportsColoredFrames && (f.getBitsPerPixel() > maxbitsperpixel)) {
		BOOST_LOG_TRIVIAL(debug) << "[GenericWriter] storing frames with more than " << maxbitsperpixel << " not supported";
		return f;
	}

	if (!ignore_duplicates) {
		uint32_t checksum = f.getChecksum();
		if (seen.contains(checksum)) {
			BOOST_LOG_TRIVIAL(trace) << "[GenericWriter] frame seen before, ignoring";
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

void GenericWriter::close()
{
	if (!frames_to_write.empty()) {
		BOOST_LOG_TRIVIAL(info) << "[GenericWriter] writing " << frames_to_write.size() << " frames";
	}
	while (!frames_to_write.empty()) {
		auto nextFrame = frames_to_write.front();
		writeFrameToFile(nextFrame.first, nextFrame.second);
		frames_to_write.pop();
	}
	BOOST_LOG_TRIVIAL(debug) << "[GenericWriter] " << frameno << " frames written";
}

void GenericWriter::writeFrameToFile(DMDFrame& f, uint32_t timestamp)
{
	BOOST_LOG_TRIVIAL(debug) << "[GenericWriter] writeFrameToFile not implemented";
}
