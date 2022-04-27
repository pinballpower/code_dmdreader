#include <filesystem>
#include <regex>
#include <iostream>
#include <map>
#include <iterator>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "pubcapture.h"
#include "../util/image.h"
#include "../util/bmp.h"

using namespace std;

bool PubCapture::loadTriggers(int bitsperpixel, string directory, std::optional <DMDPalette> palette)
{
    regex file_expr("([0-9]+).bmp");

    std::filesystem::path folder(directory);
    if (!std::filesystem::is_directory(folder))
    {
        BOOST_LOG_TRIVIAL(error) << folder.string() + " is not a directory";
        return false;
    }

    map<int, RGBBuffer> rgbdata;

    // find highest ID
    int max_index = 0;
    for (const auto& entry : std::filesystem::directory_iterator(folder))
    {
        const auto full_name = entry.path().string();

        if (entry.is_regular_file())
        {
            const auto filename = entry.path().filename().string();

            smatch match;
            if (regex_search(filename, match, file_expr)) {
                int i = stoi(match.str(1));
                if (i > max_index) { max_index = i; };

                RGBBuffer buff = readBMP(full_name);
                rgbdata.insert(pair<int, RGBBuffer>(i, buff));

                BOOST_LOG_TRIVIAL(debug) << "loaded " << filename;

            }
        }
    }

    if (max_index <= 0) {
        BOOST_LOG_TRIVIAL(error) << "Couldn't find any usuable files in " << directory;
        return false;
    }

    // Find the correct palette for these if it's not given
    if (! palette) {
        vector<DMDPalette> palettes = default_palettes();
        for (const auto p : palettes) {

            BOOST_LOG_TRIVIAL(debug) << "Checking palette " << p.name;
            bool matches = true;

            map<int, RGBBuffer>::iterator itr;
            for (itr = rgbdata.begin(); itr != rgbdata.end(); ++itr) {
                RGBBuffer buff = itr->second;

                if (!p.matches(buff)) {
                    break;
                    matches = false;
                }
            }
            if (matches) {
                palette = p;
                break;
            }
        }
    }

    if (! palette) {
        BOOST_LOG_TRIVIAL(error) << "Couldn't find matching color palette for images in " << directory;
        return false;
    }

    // create masked frames
    map<int, RGBBuffer>::iterator itr;
    for (itr = rgbdata.begin(); itr != rgbdata.end(); ++itr) {
        int i = itr->first;
        RGBBuffer buf = itr->second;

        MaskedDMDFrame mf = MaskedDMDFrame();
        mf.read_from_rgbimage(buf, palette.value(), bitsperpixel);
        trigger_frames.insert(pair<int, MaskedDMDFrame>(i, mf));
    }

    BOOST_LOG_TRIVIAL(info) << "loaded files from " << directory << " configured triggers up to " << max_index;
    return true;
}

bool PubCapture::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
    string dir = pt_source.get("directory", "");
    if (dir == "") {
        BOOST_LOG_TRIVIAL(error) << "pubcapture directory has not been configured";
        return false;
    }

    int bitsperpixel = pt_general.get("bitsperpixel", 0);
    if (!bitsperpixel) {
        BOOST_LOG_TRIVIAL(error) << "couldn't detect bits/pixel";
        return false;
    }

    return loadTriggers(bitsperpixel, dir, std::nullopt); // let the system find the correct palette
}

DMDFrame PubCapture::processFrame(DMDFrame &f)
{
    // check all maksedframes if one matches
    map<int, MaskedDMDFrame>::iterator itr;
    for (auto p: trigger_frames) {
        int i = p.first;
        MaskedDMDFrame mf = p.second;

        if (mf.matches(f)) {
            cout << "found pubcapture match: " << i << "\n";
        }
    }

    return f;
}

