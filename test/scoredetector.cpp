#include "scoredetector.hpp"

#include <filesystem>

#include "../dmd/dmdframe.hpp"
#include "../util/image.hpp"
#include "../dmd/palette.hpp"
#include "../processor/patternmatcher.hpp"

void testScoreDetect()
{
	vector<uint32_t> pd_4_ffc300_data{
	0x00000000, 0x100c0000, 0x21190000, 0x32260000,
	0x43330000, 0x54400000, 0x654d0000, 0x765a0000,
	0x87670000, 0x98740000, 0xa9810000, 0xba8e0000,
	0xcb9b0000, 0xdca80000, 0xedb50000, 0xffc30000 };
	DMDPalette pd_4_ffc300 = DMDPalette(pd_4_ffc300_data, 4, "pd_4_ffc300");

	std::filesystem::current_path("/home/matuschd/code_dmdreader");
	RGBBuffer rgbbuff = RGBBuffer::fromImageFile("output/frame000558.png");
	DMDFrame f1 = DMDFrame(rgbbuff).removeColors(4, pd_4_ffc300, false);

	PatternMatcher pm = PatternMatcher("samples/csdigits1-0123456789.png");

	rgbbuff = RGBBuffer::fromImageFile("samples/csdigits1-0123456789.png", true);
	RGBBuffer section = rgbbuff.getRegion(0, 0, 4, 8);
	DMDFrame f2 = DMDFrame(section);

	f2 = f2.removeColors(4, pd_4_ffc300, true);

	for (int x = 0; x < f1.getWidth() - f2.getWidth(); x++) {
		for (int y = 0; y < f1.getHeight() - f2.getHeight(); y++) {
			auto match = pm.matchAt(f1, x, y);
			if (match.has_value()) {
				printf("%i,%i: %c\n", x, y, match.value());
			}
		}
	}

	auto fxx = f1;
}
