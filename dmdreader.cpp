// DMDReader.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <filesystem>
#include <map>
#include <stdlib.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "DMDReader.h"
#include "dmd/dmdframe.h"
#include "dmd/pubcapture.h"
#include "dmdsource/dmdsource.h"
#include "render/framerenderer.h"
#include "render/raylibrenderer.h"
#include "util/objectfactory.h"

#include "colorize/palcoloring.h"
#include "colorize/vnianimationset.h"

using namespace std;

DMDSource* source = NULL;
vector<DMDFrameProcessor*> processors = vector<DMDFrameProcessor*>();
vector<FrameRenderer*> renderers = vector<FrameRenderer*>();

bool read_config(string filename) {

	int i = 0;

	boost::property_tree::ptree pt;
	try {
		boost::property_tree::json_parser::read_json(filename, pt);
	}
	catch (const boost::property_tree::json_parser::json_parser_error e) {
		BOOST_LOG_TRIVIAL(fatal) << "couldn't parse JSON configuration file " << e.what() << ", aborting";
		exit(1);
	}

	//
	// General
	//
	boost::property_tree::ptree pt_general = pt.get_child("general");
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

	if (pt_general.get("cwd_to_configdir", false)) {
		filesystem::current_path(filesystem::path(filename).parent_path());
	}

	//
	// Sources
	//
	bool source_configured = false;
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("source")) {
			if (source_configured) {
				BOOST_LOG_TRIVIAL(info) << "ignoring " << v.first << " only a single source is supported";
			}
			else {
				source = createSource(v.first);
				if (source) {
					if (source->configure_from_ptree(pt_general, v.second)) {
						BOOST_LOG_TRIVIAL(info) << "successfully initialized input type " << v.first;
						source_configured = true;
					}
					else {
						BOOST_LOG_TRIVIAL(warning) << "couldn't initialise source " << v.first << ", ignoring";
					}
				}
				else {
					BOOST_LOG_TRIVIAL(warning) << "don't know input type " << v.first << ", ignoring";
				}
			}
		}
	}
	catch (const boost::property_tree::ptree_bad_path& e) {}

	if (!(source_configured)) {
		BOOST_LOG_TRIVIAL(error) << "couldn't initialise any source, aborting";
		return false;
	}

	// 
	// Sanity checks
	//
	int bpp_configured = pt_general.get("bitsperpixel", 0);
	SourceProperties sourceprop = source->get_properties();

	if (!(bpp_configured)) {
		pt_general.put("bitsperpixel", sourceprop.bitsperpixel);
	}
	else if (sourceprop.bitsperpixel && (bpp_configured != sourceprop.bitsperpixel)) {

		BOOST_LOG_TRIVIAL(error) << "bits/pixel configured=" << bpp_configured << ", detected=" << sourceprop.bitsperpixel <<
			" do not match, aborting";
		return false;
	}

	int width_configured = pt_general.get("columns", 0);
	if (!(width_configured)) {
		pt_general.put("columns", sourceprop.width);
	}
	else if (sourceprop.width && (width_configured != sourceprop.width)) {

		BOOST_LOG_TRIVIAL(error) << "columns configured=" << width_configured << ", detected=" << sourceprop.width <<
			" do not match, aborting";
		return false;
	}

	int height_configured = pt_general.get("rows", 0);
	if (!(height_configured)) {
		pt_general.put("rows", sourceprop.height);
	}
	else if (sourceprop.height && (height_configured != sourceprop.height)) {

		BOOST_LOG_TRIVIAL(error) << "height configured=" << width_configured << ", detected=" << sourceprop.height <<
			" do not match, aborting";
		return false;
	}


	//
	// Processors
	//
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("processor")) {
			DMDFrameProcessor* proc = createProcessor(v.first);
			if (proc) {
				if (proc->configure_from_ptree(pt_general, v.second)) {
					BOOST_LOG_TRIVIAL(info) << "successfully initialized processor " << v.first;
					processors.push_back(proc);
				}
				else {
					delete proc;
				}
			}
			else {
				BOOST_LOG_TRIVIAL(error) << "don't know processor type " << v.first << ", ignoring";
			}
		}
	}
	catch (const boost::property_tree::ptree_bad_path& e) {
		BOOST_LOG_TRIVIAL(info) << "no processors defined";
	}


	//
	// Renderers
	//
	try {
		BOOST_FOREACH(const boost::property_tree::ptree::value_type & v, pt.get_child("renderer")) {
			FrameRenderer* renderer = createRenderer(v.first);
			if (renderer) {
				if (renderer->configure_from_ptree(pt_general, v.second)) {
					BOOST_LOG_TRIVIAL(info) << "successfully initialized renderer " << v.first;
					renderers.push_back(renderer);
				}
				else {
					delete renderer;
				}
			}
			else {
				BOOST_LOG_TRIVIAL(error) << "don't know renderer type " << v.first << ", ignoring";
			}
		}
	}

	catch (const boost::property_tree::ptree_bad_path& e) {
		BOOST_LOG_TRIVIAL(info) << "no renderers defined";
	}

	return true;
}


int main(int argc, char** argv)
{
	boost::log::core::get()->set_filter
	(
		boost::log::trivial::severity >= boost::log::trivial::trace
	);

	BOOST_LOG_TRIVIAL(trace) << "[dmdreader] cwd: " << filesystem::current_path();

	string config_file;
	if (argc >= 2) {
		config_file = argv[1]; 
	}
	else {
		config_file = "dmdreader.json";
	}

	if (!read_config(config_file)) {
		BOOST_LOG_TRIVIAL(error) << "[dmdreader]couldn't configure DMDReader, aborting";
		exit(1);
	}

	int frameno = 0;
	while (!(source->finished())) {

		BOOST_LOG_TRIVIAL(trace) << "[dmdreader] processing frame " << frameno;

		DMDFrame frame = source->next_frame();

		assert(frame.is_valid());

		for (DMDFrameProcessor* proc : processors) {
			frame = proc->process_frame(frame);
			assert(frame.is_valid());
		}

		for (FrameRenderer* renderer : renderers) {
			renderer->render_frame(frame);
			assert(frame.is_valid());
		}

		frameno++;
	}

	return 0;
}
