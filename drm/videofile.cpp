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


static int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
	int err = 0;

	ctx->hw_frames_ctx = NULL;
	// ctx->hw_device_ctx gets freed when we call avcodec_free_context
	if ((err = av_hwdevice_ctx_create(&ctx->hw_device_ctx, type,
		NULL, NULL, 0)) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to create specified HW device.";
		return err;
	}

	return err;
}


VideoFile::VideoFile(const string filename, bool preparse)
{
	this->filename = filename;

	if (avformat_open_input(&inputContext, filename.c_str(), NULL, NULL) != 0) {
		BOOST_LOG_TRIVIAL(debug) << "[videofile] cannot open input file " << filename;
		return;
	}

	playbackState = VideoPlaybackState::OPENED;

	if (preparse) {
		parseStreams();
	}
}

void VideoFile::parseStreams() {

	if (playbackState == VideoPlaybackState::PARSED) {
		return;
	}

	if (avformat_find_stream_info(inputContext, NULL) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videofile] cannot find input stream information.";
		return;
	}

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

	BOOST_LOG_TRIVIAL(trace) << "[videofile] opened " << filename;
	playbackState = VideoPlaybackState::PARSED;
}

void VideoFile::connectToDecoder() {

	if (playbackState == VideoPlaybackState::DECODER_CONNECTED) {
		return;
	}

	if (playbackState != VideoPlaybackState::PARSED) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] can't connect to decoder as the video has not been parsed";
		return;
	}

	if (hw_decoder_init(decoderContext, AV_HWDEVICE_TYPE_DRM) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] couldn't initialize HW decoder";
		return;
	}

	decoderContext->thread_count = 3; // should not be needed here as it should run on hardware

	if (avcodec_open2(decoderContext, decoder, NULL) < 0) {
		BOOST_LOG_TRIVIAL(error) << "[videoplayer] failed to open codec for stream #" << videoStream;
		return;
	}

	playbackState = VideoPlaybackState::DECODER_CONNECTED;
}

void VideoFile::close()
{
	avcodec_free_context(&decoderContext);
	avformat_close_input(&inputContext);
}

bool VideoFile::seek(int64_t timeStamp, int64_t range)
{
	int64_t min_ts = timeStamp - range;
	int64_t max_ts = timeStamp + range;
	return avformat_seek_file(inputContext, videoStream, min_ts, timeStamp, max_ts, 0);
}

bool VideoFile::nextFrame(AVPacket* packet)
{
	return av_read_frame(inputContext, packet) >= 0;
}


VideoFile::~VideoFile()
{
	close();
}

VideoPlaybackState VideoFile::getPlaybackState() const
{
	return playbackState;
}
