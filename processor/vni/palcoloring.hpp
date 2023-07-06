#pragma once

#include <cstdint>
#include <cstring>
#include <map>
#include <vector>
#include <optional>

#include "palpalette.hpp"
#include "palmapping.hpp"

using namespace std;

class PalColoring {
public:
	string filename;
	int version = 1; // 1 = FSQ, 2 = VNI 
	vector<vector <uint8_t>> masks;
	int default_palette_index = -1;
	int num_palettes = 0;

	PalColoring();
	PalColoring(string filename);
	const Palette getPalette(uint32_t index) const;
	const Palette getDefaultPalette() const;
	std::unique_ptr<PaletteMapping> findMapping(uint32_t checksum) const;

	const string debugChecksums() const; // export all checksums as a string with one checksum per line

protected:
	map<uint16_t, PalPalette> palettes;
	map<uint32_t, PaletteMapping> mappings;

};