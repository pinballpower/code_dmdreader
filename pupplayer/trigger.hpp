#pragma once

#include <string>

using namespace std;

enum class TriggerLoop {
    DEFAULT,
    LOOP,
    LOOP_FILE,
    SKIP_SAME_PRIORITY,
    STOP_FILE,
    SPLASH_RESUME,
    SET_BACKGROUND
};

class PUPTrigger {

public:

    PUPTrigger();
    PUPTrigger(string configLine);

    bool isValid();

    int id = -1;
    bool active;
    string description;
    string trigger;
    int screennum=-1;
    string playlist;
    string playfile;
    int volume;         // unsupported
    int priority;
    int length;         // unsupported
    int counter;        // 
    int rest_seconds;
    TriggerLoop loop;
    int defaults;
};