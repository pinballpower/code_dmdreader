#include "pividtest.hpp"
#include "../pupplayer/pivid.hpp"

void testPIVID() {

	PIVID pivid;
	pivid.startServer("/home/matuschd/code_dmdreader");
	auto d = pivid.getDuration("resized/match/match-1728x880.mp4");
	d = pivid.getDuration("resized/match/match-1728x880.mp4");
	d = pivid.getDuration("resized/match/match-1728x880.mp4");
}