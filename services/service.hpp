#pragma once

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.hpp"

using namespace std;

class Service {

public:
	virtual bool configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source);

	virtual string name();

	virtual bool start();
	virtual void stop();

	/// <summary>
	/// A generic interface to send a command to a service
	/// </summary>
	/// <param name="cmd">the command</param>
	/// <returns>the result of this command</returns>
	virtual string command(const string& cmd); 
};
