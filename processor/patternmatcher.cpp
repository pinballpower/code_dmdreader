#include <filesystem>

#include <boost/log/trivial.hpp>


#include "patternmatcher.hpp"
#include "../util/image.hpp"
#include "../dmd/palette.hpp"

PatternMatcher::PatternMatcher()
{
	name = "PatternMatcher";
}

PatternMatcher::PatternMatcher(string patternFile)
{
	std::filesystem::path path(patternFile);
	auto fileBase = path.filename();
	auto basename = fileBase.replace_extension("").string();
	auto ext = fileBase.extension().string();

	bool split = false;
	int numCharacters = 1;
	string characters = "X";

	// it mit be a file with multiple patterns (in format *-ccccccccccc.png)
	if (basename.find("-") != string::npos) {
		split = true;
		name = basename.substr(0, basename.find("-"));
		characters = basename.substr(basename.find("-") + 1);
		numCharacters = characters.length();
	}
	else {
		name = basename;
	}
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
