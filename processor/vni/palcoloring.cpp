#include <iostream>
#include <fstream>
#include <cstdint>
#include <filesystem>

#include <boost/log/trivial.hpp>

#include "palcoloring.hpp"
#include "palmapping.hpp"
#include "streamhelper.hpp"

using namespace std;

/// <summary>
/// Default constructor, does nothing. Not to be used.
/// </summary>
PalColoring::PalColoring()
{
}

/// <summary>
/// Initialize from a .pal color palette file
/// </summary>
/// <param name="filename">File to read</param>
PalColoring::PalColoring(string filename)
{
	ifstream is;

	if (!(filesystem::exists(filename))) {
		BOOST_LOG_TRIVIAL(error) << "[coloring] file " << filename << " does not exist";
		return;
	};

	auto file_len = std::filesystem::file_size(filename);
	BOOST_LOG_TRIVIAL(trace) << "[coloring] file " << filename << " length " << file_len << " bytes";
	is.open(filename, ios::binary);

	this->filename = filename;
	version = read_u8(is);
	BOOST_LOG_TRIVIAL(trace) << "[coloring] offset " << is.tellg() << " read version as " << version;

	num_palettes = read_u16_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[coloring] offset " << is.tellg() << " read number of palettes as " << num_palettes;

	for (int i = 0; i < num_palettes; i++) {
		PalPalette p = PalPalette(is);
		palettes[p.index] = p;
		if ((default_palette_index == 0) && (p.isDefault())) {
			default_palette_index = (uint8_t)i;
		}
	}

	if (default_palette_index < 0) {
		BOOST_LOG_TRIVIAL(trace) << "[coloring] no default palette defined, using first one";
		default_palette_index = 0;
	}

	int avail = file_len - is.tellg();
	if (avail<=0) {
		is.close();
		return;
	}

	int num_mappings = read_u16_be(is);
	BOOST_LOG_TRIVIAL(trace) << "[coloring] offset " << is.tellg() << " read number of mappings as " << num_mappings;

	avail = file_len - is.tellg();
	if (avail < PaletteMapping::LENGTH * num_mappings) {
		BOOST_LOG_TRIVIAL(warning) << "[coloring] needed " << PaletteMapping::LENGTH * num_mappings << "bytes of mapping data, but only " << avail << " available, aborting.";
		is.close();
		return;
	}

	if (num_mappings > 0) {
		for (int i = 0; i < num_mappings; i++) {
			PaletteMapping mapping = PaletteMapping(is);
			mappings[mapping.checksum]=mapping;
		}
	}
	else if (avail > 0) {
		BOOST_LOG_TRIVIAL(warning) << "[coloring] no mappings defined, but still " << avail << "bytes in the file!";
		is.close();
		return;
	}

	int num_masks = read_u8(is);
	BOOST_LOG_TRIVIAL(trace) << "[coloring] offset " << is.tellg() << " read number of masks as " << num_masks;
	if (num_masks > 0) {
		int mask_bytes = (int)(file_len-is.tellg()) / num_masks;

		if (mask_bytes != 256 && mask_bytes != 512 && mask_bytes != 1536) {
			BOOST_LOG_TRIVIAL(warning) << "[coloring] " << num_masks << " with " << mask_bytes << "bytes/masks remaining, unknown size, ignoring";
			is.close();
			return;
		}

		for (int i = 0; i < num_masks; i++) {
			vector <uint8_t> mask = vector <uint8_t>(mask_bytes);
			is.read((char*)&mask[0], mask_bytes);
			masks.push_back(mask);
		}

		BOOST_LOG_TRIVIAL(trace) << "[coloring] read " << masks.size() << " masks with " << mask_bytes << " bytes per mask";
	}

	avail = file_len - is.tellg();
	if (avail) {
		BOOST_LOG_TRIVIAL(warning) << "[coloring] erad error, finished reading, but still " << avail << " bytes left";
	}

	is.close();
}

const Palette PalColoring::getPalette(uint32_t index) const
{
	return palettes.at(index);
}

const Palette PalColoring::getDefaultPalette() const
{
	return palettes.at(default_palette_index);
}

const std::optional<PaletteMapping> PalColoring::find_mapping(uint32_t checksum) const
{
	try {
		return mappings.at(checksum);
	} catch(out_of_range e) {
		return std::nullopt;
	}
}
