#include "ledmatrixrenderer.hpp"

#include "../external/rpi-rgb-led-matrix/include/led-matrix-c.h"
;

LEDMatrixRenderer::LEDMatrixRenderer() {
    name = "LEDMatrixRenderer";
}

LEDMatrixRenderer::~LEDMatrixRenderer() {
    close();
}

void LEDMatrixRenderer::renderFrame(DMDFrame& f) {

    if (f.getBitsPerPixel() < 24) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] only 24/32-bit supported, ignoring frame";
    }

    bool is32 = (f.getBitsPerPixel() == 32);

    auto pixeldata = f.getPixelData();
    auto frame_width = f.getWidth();

    int x = 0;
    int y = 0;

    for (auto px = pixeldata.begin(); px < pixeldata.end(); ) {

        uint8_t r = *px;
        px++;
        uint8_t g = *px;
        px++;
        uint8_t b = *px;
        px++;
        if (is32)
            px++;

        if (rotate_180) {
            led_canvas_set_pixel(offscreen_canvas, width-x-1, height-y-1, r, g, b);
        }
        else {
            led_canvas_set_pixel(offscreen_canvas, x, y, r, g, b);
        }

        x++;
        if (x >= frame_width) {
            x = 0;
            y++;
        }
    }
    offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
}

void LEDMatrixRenderer::close()
{
    if (matrix != NULL) {
        led_matrix_delete(matrix);
        matrix = NULL;
    }
}

bool LEDMatrixRenderer::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_renderer) {

    struct RGBLedMatrixOptions options;
    struct RGBLedRuntimeOptions rt_options;
    
    width = pt_renderer.get("width", 128);
    height = pt_renderer.get("height", 32);
    int pwm_bits = pt_renderer.get("pwm_bits", 8);
    string rgb_sequence = pt_renderer.get("rgb_sequence", "rgb");

    int multiplexing = pt_renderer.get("multiplexing", 0);
    if ((multiplexing < 0) || (multiplexing>18)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] multiplexing range is 0-18, ignoring " << multiplexing;
            multiplexing = 0;
    }

    int scanmode = pt_renderer.get("scan_mode", 0);
    if ((scanmode < 0) || (scanmode > 1)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] scanmode can only by 0/1, ignoring " << scanmode;
        scanmode = 0;
    }

    int dither_bits = pt_renderer.get("dither_bits", 0);
    if ((dither_bits < 0) || (dither_bits > 2)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] dither_bits can only by 0-2, ignoring " << dither_bits;
        dither_bits = 0;
    }

    int lsb_nanoseconds = pt_renderer.get("lsb_nanoseconds", 100);
    if ((lsb_nanoseconds < 50) || (lsb_nanoseconds > 300)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] lsb_nanoseconds range is 50-300, ignoring " << lsb_nanoseconds;
        lsb_nanoseconds = 100;
    }

    int brightness = pt_renderer.get("brightness", 100);
    if ((brightness < 0) || (brightness > 100)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] brightness range is 0-100, ignoring " << brightness;
        brightness = 100;
    }

    int row_addr_type = pt_renderer.get("row_address_type", 0);
    if ((row_addr_type < 0) || (row_addr_type > 4)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] row_address_type can be only is 0-4, ignoring " << brightness;
        row_addr_type = 100;
    }

    int max_refresh_rate = pt_renderer.get("max_refresh_rate", 60);
    if ((max_refresh_rate < 20) || (max_refresh_rate > 200)) {
        BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] max_refresh_rate allowed range is 20-200Hz, ignoring " << brightness;
        max_refresh_rate = 60;
    }

    rotate_180 = pt_renderer.get("rotate_180", false);

    memset(&options, 0, sizeof(options));
    options.rows = height;
    options.cols = width;
    options.chain_length = 1;
    options.pwm_bits = pwm_bits;
    options.hardware_mapping = "dmdreader";
    options.led_rgb_sequence = rgb_sequence.c_str();
    options.scan_mode = scanmode;
    options.multiplexing = multiplexing;
    options.pwm_dither_bits = dither_bits;
    options.pwm_lsb_nanoseconds = lsb_nanoseconds;
    options.brightness = brightness;
    options.row_address_type = row_addr_type;
    options.limit_refresh_rate_hz = max_refresh_rate

    memset(&rt_options, 0, sizeof(rt_options));
    rt_options.gpio_slowdown = 4;

    matrix = led_matrix_create_from_options_and_rt_options(&options, &rt_options);
    if (matrix == NULL)
        return false;

    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

    led_canvas_get_size(offscreen_canvas, &width, &height);

    BOOST_LOG_TRIVIAL(info) << "[ledmatrixrenderer] configured renderer for "<< width << "x" << height << " pixels";

    return true;
}
