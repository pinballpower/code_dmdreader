#pragma once

#include <string>

#include "../drm/drmhelper.hpp"

using namespace std;

class PUPScreen {

public:

    PUPScreen(string configLine);

    bool isValid();

    int screenNum = -1;
    string screenDescription;
    string playList;
    string playFile;
    string loopit;
    bool active;
    int priority;

    int parentScreen = -1;
    float x1;
    float y1;
    float x2;
    float y2;
};