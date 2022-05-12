#pragma once

#include <string>

using namespace std;

class PUPTrigger {

public:

    PUPTrigger(string configLine);

    int id = -1;
    bool active;
    string description;
    string trigger;
    int screennum;
    string playlist;
    string playfile;
    int volume;         // unsupported
    int priority;
    int length;         // unsupported
    int counter;        // 
    int rest_seconds;
    string loop;
    int defaults;
};