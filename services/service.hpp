#pragma once

#include <map>

#include <boost/property_tree/ptree.hpp>

#include "../dmd/dmdframe.hpp"

enum class ServiceResponse {
	OK, 
	ERROR,
	SERVICE_NOT_FOUND
};

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
	virtual std::pair<ServiceResponse, string> command(const string& cmd);

	void setId(int serviceId);

private:
	int serviceId = 0;
};
