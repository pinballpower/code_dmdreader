#pragma once

#include <string>
#include <thread>
#include <map>

#include <boost/process.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/beast/http.hpp>

#include "json.hpp"


using json = nlohmann::json;

using namespace std;

class PIVID {

public:
	PIVID();
	~PIVID();

	void startServer(string mediaDirectory);

	boost::property_tree::ptree sendRequest(string target, boost::beast::http::verb requestType = boost::beast::http::verb::get, string body="");
	boost::property_tree::ptree getFileMeta(string filename);
	float getDuration(string filename);

private:
	boost::process::child pividProcess;
	map<string, float> duration;
};