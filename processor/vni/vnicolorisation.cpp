#include <filesystem>

#include "vnicolorisation.hpp"
#include "animationset.hpp"
#include "animation.hpp"
#include "palmapping.hpp"

#include "palette_colorizer.hpp"
#include "../../util/crc32.hpp"

string to_hex(const vector<uint8_t> data) {
	string res;
	for (auto x : data) {
		std::ostringstream oss;
		oss << std::hex << std::setw(2) << std::setfill('0') << (unsigned)x;
		res += oss.str();
	}
	return res;
}

bool VNIColorisation::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	bool ok = false;

	string basename = pt_source.get("basename", "");
	if (basename == "") {
		BOOST_LOG_TRIVIAL(error) << "[vnicolorisation] basename has not been configured, aborting";
		return false;
	}

	if (std::filesystem::exists(basename + ".pal") && std::filesystem::exists(basename + ".vni")) {
		BOOST_LOG_TRIVIAL(info) << "[vnicolorisation] using " << basename << ".pal/.vni";
	};

	coloring = PalColoring(basename + ".pal");
	animations = VniAnimationSet(basename + ".vni");

	ok = true; // TODO: fix this

	if (!ok) {
		
	}

	// count animations and frames
	src_frame_count = src_current_animation = src_current_frame = 0;
	for (auto &a : animations.get_animations()) {
		src_frame_count += a.second.size();
	}

	// Set default palette
	col_palette = coloring.get_default_palette().get_colors();

	BOOST_LOG_TRIVIAL(info) << "[vnicolorisation] loaded colorisation: " 
		<< animations.get_animations().size() << " animations, "
		<< src_frame_count << " frames, "
		<< coloring.num_palettes << " palettes";

	return true;
}

DMDFrame VNIColorisation::processFrame(DMDFrame& f)
{
	if (f.isNull()) {
		BOOST_LOG_TRIVIAL(info) << "[vnicolorisation] got NULL frame doing nothing";
		return f;
	}

	if (f.getBitsPerPixel() > 8) {
		BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] frame is already colored, doing nothing";

		return std::move(f);
	}

	uint32_t chk;
	BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] got frame " << f.getWidth() << "x" << f.getHeight() << " " << f.getBitsPerPixel() << "bpp, checksum " << f.getChecksum();
	int w = f.getWidth();
	int h = f.getHeight();
	int len = w * h;
	int plane_len = len / 8;

	std::optional<PaletteMapping> map = std::nullopt;
	bool found_mapping = true;

	// find colormapping
	for (int i = 0; i < f.getBitsPerPixel(); i++) {
		vector<uint8_t> pd = f.getPlaneData(i);

		uint32_t chk = crc32vect(pd,true);
		BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] plane " << std::hex << i << " crc32(full frame) " << chk;
		map = coloring.find_mapping(chk);

		if (map) {
			BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] found colormapping for unmasked frame";
		}
		else {
			// try to find an colormapping that matches
			for (auto mask : coloring.masks) {
				chk = crc32vect(pd, mask, true);
				BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] plane masked crc32(full frame) " << chk;
				map = coloring.find_mapping(chk);

				if (map) {
					BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] found colormapping on masked frame";
					break;
				}
			}
		}

		if (map) {
			uint16_t index = map->palette_index;

			col_palette = coloring.get_palette(index).get_colors();
			col_mode = map->mode;

			// Should the palette be used only for  specific number of frames?
			if (map->duration) {
				col_frames_left = map->duration;
				BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] switching to palette " << index << " for " << col_frames_left << " frames";
			}
			else {
				col_frames_left = -1;
				BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] switching to palette " << index;
			}

			if (map->IsAnimation()) {
				col_animation = animations.find(map->offset);
				col_anim_frame = 0;
				animation_active = true;
				BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] starting animation " << col_animation.name << " (offset " << map->offset << ")";
			}
			else if (col_mode == ModePalette) {
				// stop animation if one if running
				animation_active = false;
			}
		}

		if (map) {
			break;
		}
	}

	// Play animation
	vector<uint8_t> color_data;
	if (animation_active) {

		color_data=color_animation_frame(f, col_animation.get_frame(col_anim_frame), len);

		col_frames_left--;
		col_anim_frame++;
		if ((col_frames_left == 0) || (col_anim_frame >= col_animation.size())) {
			animation_active = false;
			col_mode = ModePalette;
		}
	}
	else {
		AnimationFrame af;
		color_data=color_animation_frame(f, af, len);
	}

	DMDFrame res = DMDFrame(w, h, 32, color_data);

	return res;
}

vector <uint8_t> VNIColorisation::color_animation_frame(const DMDFrame &src_frame, const AnimationFrame &anim_frame, int len)
{
	if (col_mode == ModeEvent) {
		BOOST_LOG_TRIVIAL(error) << "[vnicolorisation] mode EVENT not supported, ignoring";
	}

	if (col_mode == ModeLayeredColorMask) {
		BOOST_LOG_TRIVIAL(error) << "[vnicolorisation] layered color masks to do";
	}

	vector <uint8_t> res;

	bool is_animation = (col_mode != ModeEvent) && (col_mode != ModePalette);

	uint8_t src_mask = 0xff >> (8 - src_frame.getBitsPerPixel());
	uint8_t color_mask = ~src_mask & 0x7f;

	auto anim_frame_data = anim_frame.get_frame_data();
	auto animIter = anim_frame_data.cbegin();

	// loop through pixels
	for (auto src_px: src_frame.getPixelData()) {

		DMDColor c;

		if (col_mode == ModeColorMask) {
			uint8_t ani_px = *animIter;
			animIter++;
			if (ani_px & 0x80) {
				uint8_t pv2 = src_px | (ani_px & color_mask);
				c = col_palette[pv2];
			}
			else {
				c = col_palette[src_px];
			}
		}
		else if ((col_mode == ModeReplace) || (col_mode == ModeFollowReplace)) {
			uint8_t ani_px = *animIter;
			animIter++;
			if (ani_px & 0x80) {
				uint8_t pv2 = ani_px & 0x7f;
				c = col_palette[pv2];
			}
			else {
				c = col_palette[src_px];
			}
		}
		else if (col_mode == ModeLayeredColorMask || (col_mode == ModeMaskedReplace) ) {
			// Not sure if this is correct
			uint8_t ani_px = *animIter;
			animIter++;
			if (ani_px & 0x80) {
				uint8_t pv2 = src_px | (ani_px & color_mask);
				c = col_palette[pv2];
			}
			else {
				c = col_palette[src_px];
			}
		}		else if (col_mode == ModePalette) {
			c = col_palette[src_px];
		}
		else {
			BOOST_LOG_TRIVIAL(info) << "[vnicolorisation] mode " << col_mode << " not supported, aborting";
			assert(false);
		}

		res.push_back(c.r);
		res.push_back(c.g);
		res.push_back(c.b);
		res.push_back(0);
	}

	return res;
}
