#pragma once 

#include <cstdint>

#ifdef _WIN32
#define ENDIAN LITTLE
#define LITTLE_ENDIAN
#endif

void fix_uint32_endian(uint32_t* d);

