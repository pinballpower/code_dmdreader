#pragma once

#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;


// Lifecycle of the object
enum class VideoPlaybackState {
	UNINITIALIZED,
	OPENED,
	PARSED,
	DECODER_CONNECTED,
	CLOSED,
};

/// <summary>
/// A wrapper for video files that allows some pre-processing (e.g. header/stream parsing) before starting playback
/// It uses FFMPEGs av library.
/// </summary>
class VideoFile {

public:
	VideoFile(const string filename, bool preparse=true);
	~VideoFile();

	VideoPlaybackState getPlaybackState() const;

	void parseStreams();
	void connectToDecoder();
	void close();

	bool seek(int64_t timeStamp, int64_t seekRange=100);

	bool nextFrame(AVPacket* packet);

	int videoStream; 
	AVCodec* decoder = nullptr;
	AVFormatContext* inputContext = nullptr;
	AVCodecContext* decoderContext = nullptr;
	AVStream* video = nullptr;


private:
	// VideoFile can't be copied as is contains pointers to AV contexts
	VideoFile(const VideoFile&) = delete;
	VideoFile& operator=(const VideoFile&) = delete;

	string filename;
	VideoPlaybackState playbackState = VideoPlaybackState::UNINITIALIZED;

};