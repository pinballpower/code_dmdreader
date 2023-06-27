#pragma once

#include <iostream>
#include <cstdint>
#include <string>

uint32_t read_u32_be(std::istream& f);
uint16_t read_u16_be(std::istream& f);
int16_t read_int16_be(std::istream& f);
uint8_t read_u8(std::istream& f);
std::string read_string(std::istream& f, int len);

