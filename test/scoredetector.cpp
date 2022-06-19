#include "scoredetector.hpp"

#include <filesystem>

#include "../dmd/dmdframe.hpp"
#include "../util/image.hpp"

void testScoreDetect()
{
	std::filesystem::current_path("/home/matuschd/code_dmdreader");
	RGBBuffer rgbbuff = RGBBuffer::fromImageFile("output/frame000001.png");
	DMDFrame f1 = DMDFrame(rgbbuff);

	rgbbuff = RGBBuffer::fromImageFile("samples/csdigits1-0123456789.png");
	DMDFrame f2 = DMDFrame(rgbbuff);

	auto x = f1;
}
