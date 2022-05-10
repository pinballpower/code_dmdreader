#include "dmdsource.hpp"

class NullDMDSource: public DMDSource {
public:

	NullDMDSource();

	virtual DMDFrame getNextFrame();

	virtual bool isFinished() override;
	virtual bool isFrameReady() override;

	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

	virtual SourceProperties getProperties() override;

private:

	int width = 0;
	int height = 0;
	int bitsperpixel = 0;
};