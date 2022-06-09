#include "pivid.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/process.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/log/trivial.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

using namespace std;

boost::property_tree::ptree PIVID::sendRequest(string target, boost::beast::http::verb requestType, string body) {
	boost::property_tree::ptree result;
	try
	{
		const string host = "localhost";
		const string port = "31415";
		const int httpVersion = 10;

		// The io_context is required for all I/O
		net::io_context ioc;

		// These objects perform our I/O
		tcp::resolver resolver(ioc);
		beast::tcp_stream stream(ioc);

		// Look up the domain name
		auto const results = resolver.resolve(host, port);

		// Make the connection on the IP address we get from a lookup
		stream.connect(results);

		// Set up an HTTP request message
		http::request<http::string_body> req{ requestType, target, httpVersion };
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.body() = body;
		req.prepare_payload();

		// Send the HTTP request to the remote host
		http::write(stream, req);

		// This buffer is used for reading and must be persisted
		beast::flat_buffer buffer;

		// Declare a container to hold the response
		http::response<http::dynamic_body> res;

		// Receive the HTTP response
		http::read(stream, buffer, res);
		std::stringstream ss;
		ss << boost::beast::buffers_to_string(res.body().data());
		boost::property_tree::read_json(ss, result);

		// Gracefully close the socket
		beast::error_code ec;
		stream.socket().shutdown(tcp::socket::shutdown_both, ec);

		// not_connected happens sometimes
		// so don't bother reporting it.
		//
		if (ec && ec != beast::errc::not_connected)
			throw beast::system_error{ ec };

		// If we get here then the connection is closed gracefully
	}
	catch (std::exception const& e)
	{
		BOOST_LOG_TRIVIAL(error) << "[pivid] couldn't access " << target << " on pivid server";
	}
	return result;
}

boost::property_tree::ptree PIVID::getFileMeta(string filename) {
	auto res = sendRequest("/media/" + filename);
	return res.get_child("media");
}

float PIVID::getDuration(string filename) {
	if (duration.contains(filename)) {
		return duration[filename];
	}

	try {
		float d = getFileMeta(filename).get_child("duration").get_value(0.0f);
		duration[filename] = d;
		return d;
	}
	catch (...) {
		BOOST_LOG_TRIVIAL(error) << "[pivid] couldn't retrieve meta data for " << filename;
	}

	return 0;
}

PIVID::PIVID()
{
}

PIVID::~PIVID()
{
	pividProcess.terminate();
}

void PIVID::killRunningServers()
{
	system("pkill pivid_server");
}

void PIVID::startServer(string mediaDirectory) {
	killRunningServers();
	this_thread::sleep_for(chrono::milliseconds(500));
	string args = "--media_root=" + mediaDirectory;
	pividProcess = boost::process::child(boost::process::search_path("pivid_server"), args);
}

