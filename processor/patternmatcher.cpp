#include <filesystem>

#include <boost/log/trivial.hpp>

#include "patternmatcher.hpp"
#include "../util/image.hpp"
#include "../dmd/palette.hpp"

PatternMatcher::PatternMatcher()
{
}

PatternMatcher::PatternMatcher(string patternFile)
{
	std::filesystem::path path(patternFile);
	auto fileBase = path.filename();
	auto basename = fileBase.replace_extension("").string();
	auto ext = fileBase.extension().string();

	if (basename.find("-") == string::npos) {
		BOOST_LOG_TRIVIAL(error) << "[PatternMatcher] filename " << patternFile << " not in format *-*.*, aborting";
		return;
	}
	name = basename.substr(0,basename.find("-"));
	string characters = basename.substr(basename.find("-")+1);
	int numCharacters = characters.length();
	RGBBuffer img = RGBBuffer::fromImageFile(patternFile, true);

	auto palette = find_matching_palette(default_palettes(), img);
	if (!palette.has_value()) {
		BOOST_LOG_TRIVIAL(error) << "[PatternMatcher] couldn't find matching palette for " << patternFile << ", aborting";
		return;
	}

	int widthPerImage = img.width / numCharacters;

	for (int i = 0; i < numCharacters; i++) {
		auto x = i * widthPerImage;
		RGBBuffer section = img.getRegion(x, 0, widthPerImage, img.height);
		DMDFrame frame = DMDFrame(section).removeColors(palette.value().bitsperpixel, palette.value());
		char c = characters[i];
		patterns[c] = frame;
	}

	width = widthPerImage;
	height = img.height;
}

std::optional<char> PatternMatcher::matchAt(const DMDFrame& frame, int x, int y) const
{
	for (auto p : patterns) {
		if (frame.regionMatches(p.second, x, y)) {
			return p.first;
		}
	}
	return {};
}

bool PatternMatcher::hasPatterns() const
{
	return patterns.size() > 0;
}
