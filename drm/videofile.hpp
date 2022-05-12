#pragma once

#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

/// <summary>
/// A wrapper for video files that allows some pre-processing (e.g. header/stream parsing) before starting playback
/// It uses FFMPEGs av library.
/// </summary>
class VideoFile {

public:
	VideoFile(const string filename);
	~VideoFile();

	bool isReady() const;

	int videoStream; 
	AVCodec* decoder = nullptr;
	AVFormatContext* inputContext = nullptr;

private:
	// VideoFile can't be copied as is contains pointers to AV contexts
	VideoFile(const VideoFile&) = delete;
	VideoFile& operator=(const VideoFile&) = delete;

	string filename;
	bool ready = false;

};