#include "videofile.hpp"

#include <boost/log/trivial.hpp>

VideoFile::VideoFile(const string filename)
{
	this->filename = filename;

	if (avformat_open_input(&inputContext, filename.c_str(), NULL, NULL) != 0) {
		BOOST_LOG_TRIVIAL(error) << "[videofile] cannot open input file " << filename;
		return;
	}

	if (avformat_find_stream_info(inputContext, NULL) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videofile] cannot find input stream information.";
		return;
	}

	BOOST_LOG_TRIVIAL(trace) << "[videofile] opened and pre-parsed " << filename;
	ready = true;

	/* find the video stream information */
	videoStream = av_find_best_stream(inputContext, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
	if (videoStream < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] cannot find a video stream in the input file";
		return;
	}

	ready = true;
}

VideoFile::~VideoFile()
{
	avformat_close_input(&inputContext);
}

bool VideoFile::isReady() const
{
	return ready;
}
