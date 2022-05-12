#include "videofile.hpp"

#include <boost/log/trivial.hpp>

VideoFile::VideoFile(const string filename)
{
	if (avformat_open_input(&formatContext, filename.c_str(), NULL, NULL) != 0) {
		BOOST_LOG_TRIVIAL(error) << "[videofile] cannot open input file " << filename;
		return;
	}

	if (avformat_find_stream_info(formatContext, NULL) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videofile] cannot find input stream information.";
		return;
	}

	BOOST_LOG_TRIVIAL(trace) << "[videofile] opened and pre-parsed " << filename;
	ready = true;
}

VideoFile::~VideoFile()
{
	avformat_close_input(&formatContext);
}

bool VideoFile::isReady() const
{
	return ready;
}
