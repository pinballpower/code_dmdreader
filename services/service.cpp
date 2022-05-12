#include "service.hpp"

bool Service::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source)
{
	return false;
}

string Service::name()
{
	return "";
}

bool Service::start()
{
	return false;
}

void Service::stop()
{
}
