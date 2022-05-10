#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "../util/crc32.hpp"
#include "../util/image.hpp"
#include "color.hpp"

#define  PIXVAL uint32_t  // up to 32bit/pixel

using namespace std;

class DMDFrame {

protected:

	int width;
	int height;
	int bitsperpixel;
	vector<uint8_t> data;
	/// <summary>
	/// Stores bit planes
	/// </summary>
	vector<vector <uint8_t>>planes;

public:

	DMDFrame(int width = 0, int height = 0, int bitsperpixel = 0, uint8_t* data1 = nullptr, bool packed = false);
	DMDFrame(int width, int height, int bitsperpixel, vector <uint8_t> data1);
	DMDFrame(const RGBBuffer rgbBuffer);
	~DMDFrame();

	PIXVAL getPixel(int x, int y);

	bool hasSameSize(const DMDFrame &f2) const;
	bool hasSameSizeAndChecksum(const DMDFrame &f2) const;

	int getWidth() const;
	int getHeight() const;
	int getBitsPerPixel() const;
	uint8_t getPixelMask() const;
	uint32_t getChecksum(bool recalc=false) const;

	void setSize(int width, int height, int bits_per_pixel);

	const vector<uint8_t> getPixelData() const;
	const vector<uint8_t> getPlaneData(int bitno);

	void appendPixel(uint8_t px);

	bool isNull() const;
	bool isValid() const;

	string asString();

protected:

	void initMemory(int no_of_pixels = 0);
	void copyPixelData(uint8_t* dat, int len);
	void copyPackedPixelData(uint8_t* packedData, int numPixels, int bitsperpixel);
	void calculatePlanes();

	int getBytesPerPixel() const;

	// cache some stuff
	int datalen;
	int rowlen;
	uint8_t pixel_mask;

	mutable bool checksum_valid = false;

private:
	mutable uint32_t checksum;

};

