#include <iostream>
#include <assert.h> 
#include <chrono>
#include <thread>
#include <assert.h>

#include <raylib.h>

#include "framerenderer.h"
#include "raylibrenderer.h"

RaylibRenderer::RaylibRenderer() {
    width = height = px_radius = px_spacing = 0;
}

void RaylibRenderer::set_display_parameters(int width, int height, int px_radius, int px_spacing, int bitsperpixel) {

    assert((bitsperpixel > 0) && (bitsperpixel <= 8));

	this->width = width;
	this->height = height;
	this->px_radius = px_radius;
    this->px_spacing = px_spacing;
}

RaylibRenderer::~RaylibRenderer() {
	CloseWindow();
}


void RaylibRenderer::render_frame(DMDFrame &f) {

    bool use_palette = true;

    int bpp = f.get_bitsperpixel();
    bool has_alpha= (bpp == 32);

    // if the frame contains 32-bit data, these are already colored, no palette is needed anymore
    if ((bpp == 32) || (bpp == 24)) {
        use_palette = false;
    }

    BeginDrawing();
    ClearBackground(BLACK);

    int c_y = px_radius + px_spacing;
    int c_x = 0;

    int max_r = f.get_height();
    int max_c = f.get_width();
    uint32_t pixel_mask = f.get_pixelmask();

    vector<uint8_t> data = f.get_data();
    auto pxIter = data.begin();

    for (int r = 0; r < max_r; r++) {
        c_x = px_radius + px_spacing;

        for (int c = 0; c < max_c; c++) {

            Color raylibcolor;

            if (use_palette) {
                uint8_t pv = *pxIter;
                pxIter++;
                if (pv < palette.size()) {
                    DMDColor dmdc = palette.colors[pv];
                    raylibcolor.r = dmdc.c.cols.r;
                    raylibcolor.g = dmdc.c.cols.g;
                    raylibcolor.b = dmdc.c.cols.b;
                }
                else {
                    raylibcolor.r = 0;
                    raylibcolor.g = 0;
                    raylibcolor.b = 0;
                }
            }
            else {
                raylibcolor.r = *pxIter;
                pxIter++;
                raylibcolor.g = *pxIter;
                pxIter++;
                raylibcolor.b = *pxIter;
                pxIter++;
                if (has_alpha) {
                    pxIter++;
                }
            }

            DrawCircle(c_x, c_y, px_radius, raylibcolor);


            c_x += 2 * px_radius + px_spacing;
        }

        c_y += 2 * px_radius + px_spacing;
    }

    EndDrawing();
}
void RaylibRenderer::set_palette(const DMDPalette p)
{
    this->palette = palette;
}
;

void RaylibRenderer::start_display() {
    InitWindow(width, height, "DMD display");
}


bool RaylibRenderer::configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) {

    int bitsperpixel = pt_renderer.get("bitsperpixel", 0);
    if (!bitsperpixel) {
        bitsperpixel = pt_general.get("bitsperpixel", 0);
    }
    if (!bitsperpixel) {
        BOOST_LOG_TRIVIAL(error) << "couldn't detect bits/pixel";
        return false;
    }

    int rows = pt_general.get("rows", 0);
    int columns = pt_general.get("columns", 0);
    if ((rows <= 0) || (columns <= 0)) {
        BOOST_LOG_TRIVIAL(error) << "rows or columns not detected correctly";
        return false;
    }

    int width = pt_renderer.get("width", 1280);
    int height = pt_renderer.get("height", 320);
    int px_spacing = pt_renderer.get("spacing", 2);
    int radius = ((width / columns) - px_spacing) / 2;

    set_display_parameters(width, height, radius, px_spacing, bitsperpixel);

    start_display();

    return true;
}