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
#include "util/counter.hpp"
#include "util/profiler.hpp"

using namespace std;

#define COUNT_FRAMES				"main::frames::all"
#define COUNT_FRAMES_SUPPRESSED		"main::frames::supressed"
#define COUNT_FRAMES_PROCESSED		"main::frames::processes"

#define TIMING_PROCESSOR			"processor::"
#define TIMING_RENDERER			    "renderer::"

#define TIMING_FRAMES				"source::time_between_frames"
#define TIMING_DEDUPLICATED_FRAMES	"source::time_between_deduplicated_frames"

vector<std::shared_ptr<DMDSource>> sources;
vector<std::shared_ptr<DMDFrameProcessor>> processors;
vector<std::shared_ptr<FrameRenderer>> renderers;

bool terminateWhenFinished = true;

bool skip_unmodified_frames = true;
int frameEveryMicroseconds = 0; // force processing a frame every x microseconds

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

	frameEveryMicroseconds = pt_general.get("frame_every_microseconds", 0);
	if ((frameEveryMicroseconds) && skip_unmodified_frames) {
		BOOST_LOG_TRIVIAL(info) << "[readconfig] frame_every_microseconds is set, disabling skip_unmodified_frames";
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
					REGISTER_PROFILER(TIMING_PROCESSOR + proc->name, "ms");
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
					REGISTER_PROFILER(TIMING_RENDERER + renderer->name, "ms");
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
	BOOST_LOG_TRIVIAL(trace) << "[dmdreader] received quit signal, preparing exit " << filesystem::current_path();
	isFinished = true;
	terminateWhenFinished = true;
}

int64_t getMicrosecondsTimestamp() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
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

	string config_file;
	if (argc >= 2) {
		config_file = argv[1];
	}
	else {
		config_file = "/etc/dmdreader/dmdreader.json";
	}
	std::time_t t1 = std::time(nullptr);

	if (!read_config(config_file)) {
		BOOST_LOG_TRIVIAL(error) << "[dmdreader] couldn't configure DMDReader, aborting";
		exit(1);
	}

	BOOST_LOG_TRIVIAL(info) << "[dmdreader] hardware supports " << std::thread::hardware_concurrency() << " threads";

	std::time_t t2 = std::time(nullptr);
	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] loading took " << t2 - t1 << " seconds";

	int frameno = 0;
	t1 = std::time(nullptr);

	uint32_t checksum_last_frame = 0;
	bool sourcesFinished = false;
	int activeSourceIndex = 0;
	auto &source = sources[activeSourceIndex];

	// Profiling data
	REGISTER_PROFILER(TIMING_FRAMES, "ms");
	REGISTER_PROFILER(TIMING_DEDUPLICATED_FRAMES, "ms");

	auto lastMicroseconds = getMicrosecondsTimestamp();
	DMDFrame lastFrame;
	DMDFrame frame;

	while ((!(sourcesFinished) && (! isFinished))) {

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

		if (frameEveryMicroseconds) {
			if (source->isFrameReady()) {
				frame = source->getNextFrame();
				lastFrame = frame; // store a copy that can be delivered again
				lastMicroseconds = getMicrosecondsTimestamp();
				BOOST_LOG_TRIVIAL(trace) << "[dmdreader] got a new frame from the source";
			}
			else {
				auto currentTime = getMicrosecondsTimestamp();
				auto usSinceLast = (currentTime - lastMicroseconds);
				if (usSinceLast > frameEveryMicroseconds) {
					BOOST_LOG_TRIVIAL(trace) << "[dmdreader] duplicating last frame";
					lastMicroseconds = currentTime;
					frame = lastFrame;
				}
				else {
					// no new frame ready, but less than frameEveryMicroseconds time, 
					// just wait a bit and try again (wait at most 1ms, but could be shorter if
					// less time is left until next frame) 
					auto waitTime = max((int)(frameEveryMicroseconds - usSinceLast), 1000);
					std::this_thread::sleep_for(std::chrono::microseconds(waitTime));
					continue;
				}
			}
		}
		else {
			// just wait until the source provides the next frame
			DMDFrame frame = source->getNextFrame();
		}

		BOOST_LOG_TRIVIAL(trace) << "[dmdreader] processing frame " << frameno;

		INC_COUNTER(COUNT_FRAMES);
		END_PROFILER(TIMING_FRAMES);
		START_PROFILER(TIMING_FRAMES);

		if (skip_unmodified_frames) {
			if (frame.getChecksum() == checksum_last_frame) {
				INC_COUNTER(COUNT_FRAMES_SUPPRESSED);
				continue;
			}
			checksum_last_frame = frame.getChecksum();
		}
		END_PROFILER(TIMING_DEDUPLICATED_FRAMES);
		START_PROFILER(TIMING_DEDUPLICATED_FRAMES);

		assert(frame.isValid());

		for (auto &proc : processors) {
			auto name = TIMING_PROCESSOR + proc->name;
			START_PROFILER(name);
			frame = proc->processFrame(frame);
			END_PROFILER(name);

			// a processor can drop frames. In this case, it returns a null frame
			if (frame.isNull()) {
				BOOST_LOG_TRIVIAL(trace) << "[dmdreader] processor " << proc->name << " dropped frame";
			}

			assert(frame.isValid());
		}

		INC_COUNTER(COUNT_FRAMES_PROCESSED);


		for (auto &renderer : renderers) {
			auto name = TIMING_RENDERER + renderer->name;
			START_PROFILER(name);
			renderer->renderFrame(frame);
			END_PROFILER(name);
			assert(frame.isValid());
		}

		frameno++;

		if ((frameno % 100000) == 0) {
			BOOST_LOG_TRIVIAL(info) << "[dmdreader] received " << frameno / 10000 << " frames";
		}

	}

	t2 = std::time(nullptr);
	BOOST_LOG_TRIVIAL(info) << "[dmdreader] processed " << frameno << " frames in " << t2 - t1 << " seconds, "
		<< (float)frameno / (t2 - t1) << " frames/s, "
		<< source->getDroppedFrames() << " frames dropped in source";

	// if the program should not terminate, just loop endlessly auntil aborted
	while (! terminateWhenFinished) {
		BOOST_LOG_TRIVIAL(trace) << "[dmdreader] waiting for termination signal";
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	// Finishing
	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] closing sources";
	for (auto &s : sources) {
		s->close();
	}
	sources.clear();

	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] closing processors";
	for (auto &proc : processors) {
		proc->close();
	}
	processors.clear();

	BOOST_LOG_TRIVIAL(debug) << "[dmdreader] closing renderers";
	for (auto &renderer : renderers) {
		renderer->close();
	}
	renderers.clear();

	serviceRegistry.clear();

#ifdef ENABLE_REPORTING
	std::vector<std::pair<std::string, int>> allCounters = Counter::getInstance().getAllCounters();
	for (const auto& counterPair : allCounters) {
		BOOST_LOG_TRIVIAL(info) << "[report] " << counterPair.first << ": " << counterPair.second;
	}

	std::vector<std::pair<std::string, ProfilerRecord>> allProfilers = Profiler::getInstance().getAllRecords();
	for (const auto& profilerPair : allProfilers) {
		auto &data = profilerPair.second;
		BOOST_LOG_TRIVIAL(info) << "[report] " << profilerPair.first << ": "
			<< std::fixed << std::setprecision(1)
			<< data.avg() << data.unit << " avg, " << data.min << "-" << data.max << data.unit
			<< " (" << data.count << " recs)";
	}

#endif

	BOOST_LOG_TRIVIAL(info) << "[dmdreader] exiting";

	return 0;
}
