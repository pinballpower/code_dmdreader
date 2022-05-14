#pragma once

#include <string>
#include <vector>

using namespace std;

class PUPPlaylist {

public:

    PUPPlaylist();
    PUPPlaylist(const string configLine);

    bool isValid() const;
    void scanFiles(const string baseDirectory);

    string nextFile();

    int screenNum = -1;
    string folder;
    string description;
    bool alphaSort;
    int restSeconds;
    int volume;
    int priority;

    vector<string> files;
    int currentFileIndex = 0;
};