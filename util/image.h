#pragma once

#include <cstdint>
#include <vector>

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
	RGBBuffer(int width, int height);
	~RGBBuffer();

	const vector <uint8_t> get_data();

	void set_pixel(int x, int y, rgb data);
	void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

private:
	vector <uint8_t> data;

};