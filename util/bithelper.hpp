#pragma once

#include <cstdint>
#include <vector>

using namespace std;

uint8_t reverse_byte(uint8_t x);
void reverse_byte_array(uint8_t* arr, int len);
void reverse_byte_vector(vector<uint8_t> &vect);

uint32_t reverse_word_order(uint32_t x);
void reverse_word_order_array(uint32_t* arr, int len);
