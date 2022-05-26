#pragma once

#include <string>

#include "../drm/drmhelper.hpp"

using namespace std;

class PUPScreen {

public:

    PUPScreen();
    PUPScreen(string configLine);

    bool isValid() const;
    bool hasDisplay() const;

    int screenNum = -1;
    string screenDescription;
    string playList;
    string playFile;
    string loopit;
    bool active;
    int priority;

    int parentScreen = -1;

    // real screen ccordinates
    int displayNumber = -1;
    float x = 0;
    float y = 0; 
    float width = 0;
    float height = 0;

    // Coordinates on the real display that this will be displayed on
    CompositionGeometry composition;
};