#pragma once

#include <string>

extern "C" {
#include <libavformat/avformat.h>
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

private:
	bool ready = false;
	AVFormatContext* formatContext = nullptr;

};