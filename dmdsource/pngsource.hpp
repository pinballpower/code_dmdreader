#include <queue>

#include "dmdsource.hpp"

using namespace std;

class PNGSource : public DMDSource{
public:

	virtual DMDFrame getNextFrame() override;

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

private:

	queue<DMDFrame> frames;
	DMDFrame lastFrame;
	int currentFrame = 0;

	bool no_finish = 1;
};
