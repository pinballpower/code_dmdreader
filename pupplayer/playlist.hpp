#pragma once

#include <string>

using namespace std;

class PUPPlaylist {

public:

    PUPPlaylist(string configLine);

    bool isValid();

    int screenNum = -1;
    string folder;
    string description;
    bool alphaSort;
    int restSeconds;
    int volume;
    int priority;
};