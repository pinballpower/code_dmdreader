#pragma once

#include <cstdint>
#include <vector>
#include <string>

using namespace std;

typedef struct rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;


class RGBBuffer {

public:
	int width;
	int height;

	RGBBuffer();
	RGBBuffer(int width, int height, bool alpha=false);
	~RGBBuffer();

	void setPixel(int x, int y, rgb data);
	void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff);

	const vector<uint8_t> &getData() const;

	static RGBBuffer fromPNG(const string filename, bool useAlpha = false);

	bool isNull() const;

protected:
	vector <uint8_t> data;
	bool alpha = false;
	int bytesPerPixel = 0;

};
