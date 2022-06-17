#pragma once

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <queue>
#include <utility>
#include <chrono>

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.hpp"
#include "frameprocessor.hpp"

class PNGWriter : public GenericWriter {

protected:

	void writeFrameToFile(DMDFrame& f, uint32_t timestamp);
};