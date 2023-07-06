#include <filesystem>
#include <regex>
#include <iostream>
#include <map>
#include <iterator>

#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include "pupcapture.hpp"
#include "../util/image.hpp"
#include "../services/serviceregistry.hpp"

using namespace std;

bool PUPCapture::loadTriggers(int bitsperpixel, string directory, std::optional <DMDPalette> palette)
{
    regex file_expr("([0-9]+).bmp");

    std::filesystem::path folder(directory);
    if (!std::filesystem::is_directory(folder))
    {
        BOOST_LOG_TRIVIAL(error) << "[pupcapture] " << folder.string() + " is not a directory";
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

                RGBBuffer buff = RGBBuffer::fromImageFile(full_name);
                if (buff.isValid()) {
                    rgbdata.insert(pair<int, RGBBuffer>(i, buff));
                    BOOST_LOG_TRIVIAL(debug) << "[pupcapture] loaded " << filename;
                }
                else {
                    BOOST_LOG_TRIVIAL(error) << "[pupcapture] couldn't load " << filename << ", ignoring";
                }
                
            }
        }
    }

    if (max_index <= 0) {
        BOOST_LOG_TRIVIAL(error) << "[pupcapture] couldn't find any usuable files in " << directory << ", not using pupcapture";
        return false;
    }

    // Find the correct palette for these if it's not given
    if (! palette) {
        vector<DMDPalette> palettes = default_palettes();
        for (const auto p : palettes) {

            BOOST_LOG_TRIVIAL(debug) << "[pupcapture] checking palette " << p.name;
            bool matchesImage = true;

            map<int, RGBBuffer>::iterator itr;
            for (itr = rgbdata.begin(); itr != rgbdata.end(); ++itr) {
                RGBBuffer buff = itr->second;

                if (!p.matchesImage(buff)) {
                    break;
                    matchesImage = false;
                }
            }
            if (matchesImage) {
                palette = p;
                break;
            }
        }
    }

    if (! palette) {
        BOOST_LOG_TRIVIAL(error) << "[pupcapture] couldn't find matching color palette for images in " << directory << ", not using pupcapture";
        return false;
    }

    // create masked frames
    map<int, RGBBuffer>::iterator itr;
    for (itr = rgbdata.begin(); itr != rgbdata.end(); ++itr) {
        int i = itr->first;
        RGBBuffer buf = itr->second;

        MaskedDMDFrame mf = MaskedDMDFrame();
        mf.readFromRGBImage(buf, palette.value(), bitsperpixel);
        trigger_frames.insert(pair<int, MaskedDMDFrame>(i, mf));
    }

    return true;
}

bool PUPCapture::configureFromPtree(boost::property_tree::ptree pt_general, boost::property_tree::ptree pt_source) {
    string dir = pt_source.get("directory", ".");

    int bitsperpixel = pt_general.get("bitsperpixel", 0);
    if (!bitsperpixel) {
        BOOST_LOG_TRIVIAL(error) << "[pupcapture] couldn't detect bits/pixel, you need to set it in the general section of the configuration file, disabling pupcapture";
        return false;
    }

    if (loadTriggers(bitsperpixel, dir, std::nullopt)) { // let the system find the correct palette
        int masked = 0;
        int unmasked = 0;
        for (const auto& f : trigger_frames) {
            if (f.second.isMasked()) {
                masked++;
            }
            else {
                unmasked++;
            }
        }
        BOOST_LOG_TRIVIAL(info) << "[pupcapture] loaded " << trigger_frames.size() << " trigger frames (" << masked << " masked, " << unmasked << " unmasked)";
        return true;
    }
    else {
        return false;
    }
}

DMDFrame PUPCapture::processFrame(DMDFrame &f)
{
    // check all maksedframes if one matches
    map<int, MaskedDMDFrame>::iterator itr;
    for (const auto &p: trigger_frames) {
        int i = p.first;
        MaskedDMDFrame mf = p.second;

        if (mf.matchesImage(f)) {

            string trigger = "trigger:D"+ std::to_string(p.first);

            vector <std::pair<ServiceResponse, string>> res = serviceRegistry.command("pupplayer", trigger);
            if (res.size() == 0) {
                BOOST_LOG_TRIVIAL(error) << "[pupcapture] pupplayer not available, has probably not been configured";
            }
            else {
                BOOST_LOG_TRIVIAL(debug) << "[pupcapture] sent event " << trigger << " to " << res.size() << " pupplayer(s)";
            }

            break;
        }
    }

    return f;
}

PUPCapture::PUPCapture() {
    name = "PUPCapture";
}
