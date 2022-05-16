#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>
#include <map>
#include <csignal>
#include <stdlib.h>
#include <chrono>
#include <thread>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "dmd/dmdframe.hpp"
#include "dmdsource/dmdsource.hpp"
#include "util/objectfactory.hpp"
#include "services/serviceregistry.hpp"

#include "pupplayer/pupplayer.hpp"

using namespace std;

vector<std::shared_ptr<DMDSource>> sources;
vector<std::shared_ptr<DMDFrameProcessor>> processors;
vector<std::shared_ptr<FrameRenderer>> renderers;

bool terminateWhenFinished = true;

bool skip_unmodified_frames = true;

bool read_config(string filename) {

	int i = 0;

	ifstream configFile(filename);
	if (!configFile) {
		BOOST_LOG_TRIVIAL(fatal) << "[readconfig] can't read JSON configuration file " << filename << ", aborting";
		exit(1);
	}

	boost::property_tree::ptree pt;
	try {
		boost::property_tree::json_parser::read_json(filename, pt);
		BOOST_LOG_TRIVIAL(info) << "[readconfig] using configuration file " << filename;
	}
	catch (const boost::property_tree::json_parser::json_parser_error e) {
		BOOST_LOG_TRIVIAL(fatal) << "[readconfig] couldn't parse JSON configuration file " << e.what() << ", aborting";
		exit(1);
	} 

	//
	// General
	//
	boost::property_tree::ptree pt_general = pt.get_child("general");
	terminateWhenFinished = pt_general.get("terminate_when_finished", true);

	if ((pt_general.get("type", "") == "spike") || (pt_general.get("type", "") == "spike1")) {
		pt_general.put("bitsperpixel", 4);
		pt_general.put("rows", 32);
		pt_general.put("columns", 128);
	}

	if ((pt_general.get("type", "") == "wpc")) {
		pt_general.put("bitsperpixel", 2);
		pt_general.put("rows", 32);
		pt_general.put("columns", 128);
	}

	if ((pt_general.get("type", "") == "whitestar")) {
		pt_general.put("bitsperpixel", 4);
		pt_general.put("rows", 32);
		pt_general.put("columns", 128);
	}

	skip_unmodified_frames = pt_general.get("skip_unmodified_frames", true);

	if (pt_general.get("cwd_to_configdir", false)) {
		filesystem::current_path(filesystem::path(filename).parent_path());
		BOOST_LOG_TRIVIAL(debug) << "[readconfig]  set working directory to " << filesystem::current_path();
	}

	if (pt_general.get("log", "") == "debug") {
		BOOST_LOG_TRIVIAL(debug) << "[readconfig] enabling debug logging";
		boost::log::core::get()->set_filter
		(
			boost::log::trivial::severity >= boost::log::trivial::debug
		);
	}
	else if (pt_general.get("log", "") == "trace") {
		BOOST_LOG_TRIVIAL(debug) << "[readconfig] enabling trace logging";
		boost::log::core::get()->set_filter
		(
			boost::log::trivial::severity >= boost::log::trivial::trace
		);
	}
	else if (pt_general.get("log", "") == "quiet") {
		boost::log::core::get()->set_filter
		(
			boost::log::trivial::severity > boost::log::trivial::error
		);
	}

	//
	// Sources
	//
	bool source_configured = false;
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("source")) {

			std::shared_ptr<DMDSource> source = createSource(v.first);
			if (source) {
				if (source->configureFromPtree(pt_general, v.second)) {
					BOOST_LOG_TRIVIAL(info) << "[readconfig] successfully initialized input type " << v.first;
					sources.push_back(source);
				}
				else {
					BOOST_LOG_TRIVIAL(warning) << "[readconfig] couldn't initialise source " << v.first << ", ignoring";
				}
			}
			else {
				BOOST_LOG_TRIVIAL(warning) << "[readconfig] don't know input type " << v.first << ", ignoring";
			}
		}
	}
	catch (const boost::property_tree::ptree_bad_path& e) {}

	if (sources.empty()) {
		BOOST_LOG_TRIVIAL(error) << "[readconfig] couldn't initialise any source, aborting";
		return false;
	}

	//
	// Processors
	//
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("processor")) {
			std::shared_ptr<DMDFrameProcessor> proc = createProcessor(v.first);
			if (proc) {
				if (proc->configureFromPtree(pt_general, v.second)) {
					BOOST_LOG_TRIVIAL(info) << "[readconfig] successfully initialized processor " << v.first;
					processors.push_back(proc);
				}
			}
			else {
				BOOST_LOG_TRIVIAL(error) << "[readconfig] don't know processor type " << v.first << ", ignoring";
			}
		}
	}
	catch (const boost::property_tree::ptree_bad_path& e) {
		BOOST_LOG_TRIVIAL(info) << "[readconfig] no processors defined";
	}


	//
	// Renderers
	//
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("renderer")) {
			std::shared_ptr<FrameRenderer> renderer = createRenderer(v.first);
			if (renderer) {
				if (renderer->configureFromPtree(pt_general, v.second)) {
					BOOST_LOG_TRIVIAL(info) << "[readconfig] successfully initialized renderer " << v.first;
					renderers.push_back(renderer);
				}
				else {
					BOOST_LOG_TRIVIAL(info) << "[readconfig] could not initialize renderer " << v.first << ", ignoring";
				}
			}
			else {
				BOOST_LOG_TRIVIAL(error) << "[readconfig] don't know renderer type " << v.first << ", ignoring";
			}
		}
	}

	catch (const boost::property_tree::ptree_bad_path& e) {
		BOOST_LOG_TRIVIAL(info) << "[readconfig] no renderers defined";
	}

	//
	// Services
	//
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("service")) {
			std::shared_ptr<Service> service = createService(v.first);
			if (service) {
				if (service->configureFromPtree(pt_general, v.second)) {
					if (service->start()) {
						serviceRegistry.registerService(service);
						BOOST_LOG_TRIVIAL(info) << "[info] registered service " << service->name();
					}
					else {
						BOOST_LOG_TRIVIAL(info) << "[error] could not start service " << v.first;
					}
				}
				else {
					BOOST_LOG_TRIVIAL(info) << "[readconfig] could not initialize service " << v.first << ", ignoring";
				}
			}
			else {
				BOOST_LOG_TRIVIAL(error) << "[readconfig] don't know service type " << v.first << ", ignoring";
			}
		}
	}

	catch (const boost::property_tree::ptree_bad_path& e) {
		BOOST_LOG_TRIVIAL(info) << "[readconfig] no services defined";
	}

	return true;
}

volatile bool isFinished = false;

void signal_handler(int sig)
{
	isFinished = true;
	terminateWhenFinished = true;
}


int main(int argc, char** argv)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
#ifdef SIGBREAK
	signal(SIGBREAK, signal_handler);
#endif


	boost::log::core::get()->set_filter
	(
		boost::log::trivial::severity >= boost::log::trivial::info
	);

	BOOST_LOG_TRIVIAL(trace) << "[dmdreader] cwd: " << filesystem::current_path();

	DRMHelper::logResources();

	string config_file;
	if (argc >= 2) {
		config_file = argv[1];
	}
	else {
		config_file = "/etc/dmdreader/dmdreader.json";
	}
	std::time_t t1 = std::time(nullptr);

	if (!read_config(config_file)) {
		BOOST_LOG_TRIVIAL(error) << "[dmdreader]couldn't configure DMDReader, aborting";
		exit(1);
	}

	BOOST_LOG_TRIVIAL(info) << "[dmdreader] hardware supports " << std::thread::hardware_concurrency() << " threads";

	std::time_t t2 = std::time(nullptr);
	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] loading took " << t2 - t1 << " seconds";

	int frameno = 0;
	t1 = std::time(nullptr);

	uint32_t checksum_last_frame = 0;
	int skippedFrames = 0;
	bool sourcesFinished = false;
	int activeSourceIndex = 0;
	auto source = sources[activeSourceIndex];

	while ((!(sourcesFinished) && (! isFinished))) {

		BOOST_LOG_TRIVIAL(trace) << "[dmdreader] processing frame " << frameno;

		if (source->isFinished()) {
			// switch to next source if there is one
			if (activeSourceIndex < sources.size() - 1) {
				activeSourceIndex++;
				source = sources[activeSourceIndex];
			}
			else {
				sourcesFinished = true;
				continue;
			}
		}
		DMDFrame frame = source->getNextFrame();


		if (skip_unmodified_frames) {
			if (frame.getChecksum() == checksum_last_frame) {
				skippedFrames++;
				continue;
			}
			checksum_last_frame = frame.getChecksum();
		}

		assert(frame.isValid());

		for (auto proc : processors) {
			frame = proc->processFrame(frame);
			assert(frame.isValid());
		}

		for (auto renderer : renderers) {
			renderer->renderFrame(frame);
			assert(frame.isValid());
		}

		frameno++;

	}

	t2 = std::time(nullptr);
	BOOST_LOG_TRIVIAL(info) << "[dmdreader] processed " << frameno << " frames in " << t2 - t1 << " seconds, " 
		<< (float)frameno / (t2 - t1) << " frames/s, "
		<< source->getDroppedFrames() << " frames dropped, "
		<< skippedFrames << " duplicated frames skipped";

	// if the program should not terminate, just loop endlessly auntil aborted
	while (! terminateWhenFinished) {
		BOOST_LOG_TRIVIAL(trace) << "[dmdreader] waiting for termination signal";
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	// Finishing
	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] closing sources";
	for (auto s : sources) {
		s->close();
	}
	sources.clear();

	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] closing processors";
	for (auto proc : processors) {
		proc->close();
	}
	processors.clear();

	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] closing renderers";
	for (auto renderer : renderers) {
		renderer->close();
	}
	renderers.clear();

	serviceRegistry.clear();

	BOOST_LOG_TRIVIAL(info) << "[dmdreader] exiting";

	return 0;
}
