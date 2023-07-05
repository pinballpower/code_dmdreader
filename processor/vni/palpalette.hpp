#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>

#include <boost/log/trivial.hpp>

#include "../../dmd/color.hpp"

using namespace std;

class Palette {

public:
	const vector<DMDColor> get_colors() const;

protected:
	vector<DMDColor> colors;
};

class PalPalette : public Palette  {

public:
	uint16_t index;
	int type; //  0: normal, 1: default

	PalPalette();
	~PalPalette();
	PalPalette(istream& is);

	bool isDefault() const;
	bool isPersistent() const;

};