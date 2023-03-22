#include "ledmatrixrenderer.hpp"

#include "../external/rpi-rgb-led-matrix/include/led-matrix-c.h"
;

LEDMatrixRenderer::LEDMatrixRenderer() {
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
    auto width = f.getWidth();

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

        led_canvas_set_pixel(offscreen_canvas, x, y, r, b, g);

        x++;
        if (x >= width) {
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
    
    int width, height, pwm_bits;
    int x, y, i;

    width = pt_renderer.get("width", 128);
    height = pt_renderer.get("height", 32);
    pwm_bits = pt_renderer.get("pwm_bits", 8);
    
    // TODO: check what other configuration parameters could be useful

    memset(&options, 0, sizeof(options));
    options.rows = height;
    options.cols = width;
    options.chain_length = 1;
    options.hardware_mapping = "dmdreader";

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
