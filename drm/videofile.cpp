#include "videofile.hpp"

#include <boost/log/trivial.hpp>


static enum AVPixelFormat hw_pix_fmt;

static enum AVPixelFormat get_hw_format(AVCodecContext* ctx,
	const enum AVPixelFormat* pix_fmts)
{
	const enum AVPixelFormat* p;

	for (p = pix_fmts; *p != -1; p++) {
		if (*p == hw_pix_fmt)
			return *p;
	}

	BOOST_LOG_TRIVIAL(error) << "[videofile] failed to get HW surface format.";
	return AV_PIX_FMT_NONE;
}



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

	if (decoder->id == AV_CODEC_ID_H264) {
		if ((decoder = avcodec_find_decoder_by_name("h264_v4l2m2m")) == NULL) {
			BOOST_LOG_TRIVIAL(error) << "[videoplayer] cannot find the h264 v4l2m2m decoder";
			return;
		}
		hw_pix_fmt = AV_PIX_FMT_DRM_PRIME;
	}
	else {
		for (int i = 0;; i++) {
			const AVCodecHWConfig* config = avcodec_get_hw_config(decoder, i);
			if (!config) {
				BOOST_LOG_TRIVIAL(error) << "[videoplayer] decoder " << decoder->name << " does not support DRM";
				return;
			}
			if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
				config->device_type == AV_HWDEVICE_TYPE_DRM) {
				hw_pix_fmt = config->pix_fmt;
				break;
			}
		}
	}

	if (!(decoderContext = avcodec_alloc_context3(decoder))) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't allocate AV codec";
		return;
	}

	video = inputContext->streams[videoStream];
	if (avcodec_parameters_to_context(decoderContext, video->codecpar) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't get context";
		return;
	}

	decoderContext->get_format = get_hw_format;

	ready = true;


}

VideoFile::~VideoFile()
{
	avcodec_free_context(&decoderContext);
	avformat_close_input(&inputContext);
}

bool VideoFile::isReady() const
{
	return ready;
}
