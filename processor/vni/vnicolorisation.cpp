#include <filesystem>
#include <cstdint>
#include <cstring>

#include "vnicolorisation.hpp"
#include "animationset.hpp"
#include "animation.hpp"
#include "palmapping.hpp"
#include "switchmode.hpp"

#include "palette_colorizer.hpp"
#include "../../util/crc32.hpp"
#include "../../util/counter.hpp"

#define FOUND_UNMASKED		"vnicolorisation::found::unmasked"
#define FOUND_MASKED		"vnicolorisation::found::masked"
#define FOUND_NOT			"vnicolorisation::found::not_found"
#define FRAMES				"vnicolorisation::frames"
#define COL_MODE			"vnicolorisation::mode::"


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

	if (coloring.num_palettes == 0) {
		BOOST_LOG_TRIVIAL(error) << "[vnicolorisation] couldn't load any palette data from .pal file";
		return false;
	}

	if (animations.getAnimations().size() == 0) {
		BOOST_LOG_TRIVIAL(error) << "[vnicolorisation] couldn't load any animations from .vni file";
		return false;
	}

	// count animations and frames
	src_frame_count = 0;
	for (auto &a : animations.getAnimations()) {
		src_frame_count += a.second.size();
	}

	// Set default palette
	setDefaultPalette();

	BOOST_LOG_TRIVIAL(info) << "[vnicolorisation] loaded colorisation: " 
		<< animations.getAnimations().size() << " animations, "
		<< src_frame_count << " frames, "
		<< coloring.num_palettes << " palettes";

	REGISTER_COUNTER(FOUND_UNMASKED);
	REGISTER_COUNTER(FOUND_MASKED);
	REGISTER_COUNTER(FOUND_NOT);
	REGISTER_COUNTER(FRAMES);

	return true;
}

std::unique_ptr<PaletteMapping> VNIColorisation::findMapForPlaneData(const vector<uint8_t> pd) const {
	std::unique_ptr<PaletteMapping> map;

	uint32_t chk = crc32vect(pd, true);
	BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] plane crc32(full frame) " << std::hex << chk;
	map = coloring.findMapping(chk);

	if (map) {
		BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] found colormapping for unmasked frame";
		INC_COUNTER(FOUND_UNMASKED);
	}
	else {
		// try to find a colormapping that matches
		int i = 0;
		for (auto &mask : coloring.masks) {
			chk = crc32vect(pd, mask, true);
			BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] plane masked crc32(full frame) " << std::hex << chk  << " with mask " << i;
			map = coloring.findMapping(chk);

			if (map) {
				BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] found colormapping on masked frame";
				INC_COUNTER(FOUND_MASKED);
				break;
			}
			i++;
		}
		INC_COUNTER(FOUND_NOT);
		BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] no mapping found";

	}

	return map;
}

void  VNIColorisation::setPalette(const vector<DMDColor> colors) {
	previous_col_palette = std::move(col_palette);
	col_palette = colors;
}

void VNIColorisation::setPreviousPalette() {
	col_palette = std::move(previous_col_palette);
}

void VNIColorisation::setDefaultPalette() {
	previous_col_palette = std::move(col_palette);
	setPalette(coloring.getDefaultPalette().get_colors());
}

bool VNIColorisation::triggerAnimation(const DMDFrame& f) {

	std::unique_ptr<PaletteMapping> map;

	// find colormapping
	for (int i = 0; i < f.getBitsPerPixel(); i++) {
		vector<uint8_t> pd = f.getPlaneData(i);

		map = findMapForPlaneData(pd);

		if (map) {
			uint16_t index = map->palette_index;

			setPalette(coloring.getPalette(index).get_colors());
			col_mode = map->mode;

			// Should the palette be used only for  specific number of frames?
			if (map->duration) {
				col_frames_left = map->duration;
				BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] switching to palette " << index << " for " << col_frames_left << " frames";
				// TODO: Implement start/stop/isActive
			}
			else {
				col_frames_left = -1;
				BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] switching to palette " << index;
			}

			if (map->IsAnimation()) {
				auto anim = animations.find(map->offset);
				if (anim.sameAnimation(col_animation)) {
					BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] animation " << col_animation.name << "already running, " << col_animation.framesLeft() << " frames left";
				}
				else {
					col_animation = anim;
					col_animation.start();
					BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] starting animation " << col_animation.name << ", " << col_animation.framesLeft() << " frames left";
				}
			}
			else if (col_mode == ModePalette) {
				// stop animation if one if running
				col_animation.stop();
			}

			return true;
		}
	} 

	return false; // no mapping found
}

DMDFrame VNIColorisation::processFrame(DMDFrame& f)
{
	if (f.isNull()) {
		BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] got NULL frame doing nothing";
		return f;
	}

	if (f.getBitsPerPixel() > 8) {
		BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] frame is already colored, doing nothing";
		return std::move(f);
	}

	bool srcFrameChanged = (crcLastFrame != f.getChecksum());
	if (srcFrameChanged) {
		crcLastFrame = f.getChecksum();
	}

	BOOST_LOG_TRIVIAL(trace) << "[vnicolorisation] got frame " << f.getWidth() << "x" << f.getHeight() << " " << f.getBitsPerPixel() << "bpp, checksum " << f.getChecksum();
	INC_COUNTER(FRAMES);

	int w = f.getWidth();
	int h = f.getHeight();
	int len = w * h;
	int plane_len = len / 8;

	bool found_mapping = triggerAnimation(f);

	// Play animation
	vector<uint8_t> color_data;

	if (col_animation.isActive()) {
		color_data= colorAnimationFrame(f, col_animation.getNextFrame(srcFrameChanged).value(), len);
		// the animation could be finished after this frame, in this case, switch back to palette mode
		if (!col_animation.isActive()) {
			col_mode = ModePalette;
			setDefaultPalette();
		}
	} else {
		AnimationFrame af;
		color_data= colorAnimationFrame(f, af, len);
	}

	return DMDFrame(w, h, 32, color_data);
}

/*
 * Checks if this pixel in the animation should be used (Mask bit 7 is set)
 */
bool isActive(uint8_t animationPixel) {
	return animationPixel & 0x80;
}


vector <uint8_t> VNIColorisation::colorAnimationFrame(const DMDFrame &src_frame, const AnimationFrame &anim_frame, int len)
{
	if (col_mode == ModeEvent) {
		BOOST_LOG_TRIVIAL(error) << "[vnicolorisation] mode EVENT not supported, ignoring";
	}

	vector <uint8_t> res;

	uint8_t src_mask = 0xff >> (8 - src_frame.getBitsPerPixel());
	uint8_t color_mask = ~src_mask & 0x7f;

	auto anim_frame_data = anim_frame.getFrameData();
	auto animIter = anim_frame_data.cbegin();

	BOOST_LOG_TRIVIAL(debug) << "[vnicolorisation] mode " << switchModeStr(col_mode);
	INC_COUNTER(COL_MODE + switchModeStr(col_mode));


	// loop through pixels
	for (auto src_px: src_frame.getPixelData()) {

		DMDColor c;
		if (usesAnimationFrame(col_mode)) {

			uint8_t ani_px = *animIter;
			animIter++;

			if (isActive(ani_px)) {

				if (col_mode == ModeColorMask) {
					uint8_t pv2 = src_px | (ani_px & color_mask);
					c = col_palette[pv2];
				}
				else if ((col_mode == ModeReplace) || (col_mode == ModeFollowReplace)) {
					uint8_t pv2 = ani_px & 0x7f;
					c = col_palette[pv2];
				}
				else if (col_mode == ModeLayeredColorMask || (col_mode == ModeMaskedReplace)) {
					uint8_t pv2 = src_px | (ani_px & color_mask);
					c = col_palette[pv2];
				}
			}
			else {
				c = col_palette[src_px];
			}
		}		
		else if (col_mode == ModePalette) {
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
