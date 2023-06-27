#include <cstdint>
#include <string>
#include <fstream>
#include <filesystem>

#include <boost/log/trivial.hpp>

#include "vnianimationset.hpp"
#include "vnianimation.hpp"
#include "streamhelper.hpp"

using namespace std;

VniAnimationSet::VniAnimationSet() {

};


VniAnimationSet::VniAnimationSet(string filename) {

	ifstream is;

	if (!(filesystem::exists(filename))) {
		BOOST_LOG_TRIVIAL(error) << "[vnianimationset] file " << filename << " does not exist";
		return;
	};

	is.open(filename, ios::binary);

	// name
	string header = read_string(is, 4);
	if (header != "VPIN") {
		is.close();
		BOOST_LOG_TRIVIAL(error) << "[vnianimationset] file " << filename << " is not a VPIN file";
		return;
	}

	// version
	version = read_int16_be(is);

	// number of animations
	int num_animations = read_int16_be(is);

	if (version >= 2) {
		BOOST_LOG_TRIVIAL(trace) << "[vnianimationset] skipping " << num_animations*4 << "  bytes of animation indexes.";
		is.ignore(num_animations * 4);
	}

	BOOST_LOG_TRIVIAL(trace) << "[vnianimationset] reading " << num_animations << " animations from v" << version;

	max_width = 0;
	max_height = 0;

	for (int i = 0; i < num_animations; i++) {
		VniAnimation a = VniAnimation(is, version);
		BOOST_LOG_TRIVIAL(trace) << "[vnianimationset] read animation " << i;
		if (a.height > max_height)
			max_height = a.height;
		if (a.width > max_width)
			max_width = a.width;
		animations[a.offset] = a;
	}
	BOOST_LOG_TRIVIAL(trace) << "[vnianimationset] successfully loaded animations, max dimension is " << max_width << "x" << max_height;

	is.close();
}