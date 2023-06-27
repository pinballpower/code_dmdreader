#include <string>
#include "streamhelper.hpp"

using namespace std;

uint32_t read_u32_be(std::istream& f)
{
    uint32_t val;
    uint8_t bytes[4];
    f.read((char*)bytes, 4);

    val = bytes[3] | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 24);

    return val;
};

uint16_t read_u16_be(std::istream& f)
{
    uint16_t val;
    uint8_t bytes[2];
    f.read((char*)bytes, 2);

    val = bytes[1] | (bytes[0] << 8);

    return val;
};

int16_t read_int16_be(std::istream& f)
{
    int16_t val;
    uint8_t bytes[2];
    f.read((char*)bytes, 2);

    val = bytes[1] | (bytes[0] << 8);

    return val;
};


uint8_t read_u8(std::istream& f)
{
    uint8_t val;
    f.read((char*)&val, 1);
    return val;
};

/// <summary>
/// Read a number of bytes and interpret this as a string
/// </summary>
/// <param name="f">stream to read from</param>
/// <param name="len">length in bytes</param>
/// <returns></returns>
string read_string(std::istream& f, int len) {
    string result = "";
    char c;
    for (int i = 0; i < len; i++) {
        f.read(&c, 1);
        result += c;
    }
    return result;
};
