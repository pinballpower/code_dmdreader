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
#include "genericwriter.hpp"

class TXTWriter : public GenericWriter {

public:

	~TXTWriter();
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) override;

protected:

	virtual void writeFrameToFile(DMDFrame& f, uint32_t timestamp) override;
	void close();

private:
	ofstream outputfile;
};