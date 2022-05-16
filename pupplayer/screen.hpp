#pragma once

#include <string>

#include "../drm/drmhelper.hpp"

using namespace std;

class PUPScreen {

public:

    PUPScreen();
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
    float x;
    float y;
    float width;
    float height;
};