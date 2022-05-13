#pragma once

#include <string>

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
    string customPos;
};