#include "dmdsource.h"

class NullDMDSource: public DMDSource {
public:

	NullDMDSource();

	virtual unique_ptr<DMDFrame> next_frame(bool blocking = true);

	virtual bool finished();
	virtual bool frame_ready();

	virtual bool configure_from_ptree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual void get_properties(SourceProperties* p);

private:

	int width = 0;
	int height = 0;
	int bitsperpixel = 0;
};