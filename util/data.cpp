#include "data.hpp"

Rectangle::Rectangle(int x, int y, int width, int height) {
	this->x1 = x;
	this->x2 = x + width;
	this->y1 = y;
	this->y2 = y + height;
}

bool Rectangle::contains(int x, int y) const {
	return ((x >= x1) && (x < x2) && (y >= y1) && (y < y2));
};