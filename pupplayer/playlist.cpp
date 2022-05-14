#include "playlist.hpp"
#include "pupplayer.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>

PUPPlaylist::PUPPlaylist()
{
}

PUPPlaylist::PUPPlaylist(const string configLine)
{
	std::vector<std::string> fields = splitLine(configLine);

	if (fields.size() < 7) {
		BOOST_LOG_TRIVIAL(error) << "[playlist] can't parse playlist line, need at least 8 fields: \"" << configLine << "\"";
	}

	try {
		screenNum = parseInteger(fields[0]);
		folder = fields[1];
		description = fields[2];
		alphaSort = parseBool(fields[3], true);
		restSeconds = parseInteger(fields[4]);
		volume = parseInteger(fields[5]);
		priority = parseInteger(fields[6]);
	}
	catch (...) {
		BOOST_LOG_TRIVIAL(error) << "[playlist] can't parse screen line  \"" << configLine << "\"";

	}
}


bool PUPPlaylist::isValid() const
{
	return screenNum >= 0;
}

void PUPPlaylist::scanFiles(const string baseDirectory)
{
	files.clear();
	string path = baseDirectory+"/"+folder;
	for (const auto file : std::filesystem::directory_iterator(path)) {
		if (file.is_regular_file()) {
			string pathStr = file.path();
			for (const string suffix : {".mp4"}) {
				if (pathStr.ends_with(suffix)) {
					files.push_back(pathStr);
					BOOST_LOG_TRIVIAL(debug) << "[playlist] added " << pathStr << " to playlist " << folder;
					break;
				}
			}
		}
	}

	if (alphaSort) {
		std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) -> bool { return a < b; });
	}
}

string PUPPlaylist::nextFile()
{
	if (files.size() == 0) {
		return "";
	}

	if (currentFileIndex >= files.size()) {
		currentFileIndex = 0;
	}

	if (alphaSort) {
		return files[currentFileIndex];
		currentFileIndex++;
	}
	else {
		return files[rand() % files.size()];
	}
}
