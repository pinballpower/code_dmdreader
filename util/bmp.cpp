// simple BMP file reader fbased on https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string> 

#include "bmp.h"
#include "image.h"

using namespace std;

RGBBuffer read_BMP(std::string filename)
{
    ifstream is;
    is.exceptions(ifstream::failbit | ifstream::badbit);
    is.open(filename, ios::binary);

    uint8_t info[54];
    is.read((char*)info, sizeof(info)); // read the 54-byte header

    // check if it is the format with the 54 byte header
    if (info[14] != 40) {
        throw std::ios_base::failure("Can't read "+filename+", only BITMAPINFOHEADER supported, but type is "+ std::to_string(info[14]));
    }
    
    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    if ((width <= 0) || (height <= 0)) {
        cerr << "Can't read " << filename << ", invalid dimensions.\n";
        throw std::ios_base::failure("Can't read " + filename + ", invalid dimensions.");
    }

    int row_padded = (width * 3 + 3) & (~3);
    int bytesperline = 3 * width;

    uint8_t* linedata = new uint8_t[row_padded];

    RGBBuffer buf = RGBBuffer(width, height);

    for (int y = height; y ; y--)
    {
        is.read((char*)linedata, row_padded);

        for (int x=0, z = 0; x < width; x++, z+=3)
        {
            // Convert (B, G, R) to (R, G, B)
            uint8_t r = linedata[z + 2];
            uint8_t g = linedata[z + 1];
            uint8_t b = linedata[z];
            buf.set_pixel(x, y, r, g, b);
        }
    }

    delete[] linedata;

    is.close();

    // return width, height and data
    return buf;
}