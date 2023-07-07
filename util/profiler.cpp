#include "profiler.hpp"

ProfilerRecord::ProfilerRecord()
{
    startTime = 0;
}

void ProfilerRecord::addRecord(const float v)
{
    if (v < min) {
        min = v;
    }
    if (v > max) {
        max = v;
    }
    count += 1;
    sum += v;
}

float ProfilerRecord::avg() const {
    if (!count) {
        return 0;
    }
    else {
        return sum / count;
    }
}


void ProfilerRecord::startTimer() {
    // Get the current time in microseconds
    startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void ProfilerRecord::endTimer() {
    if (!startTime) return;

    auto myTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    float ms = (float)(myTime - startTime)/1000;

    addRecord(ms);
    startTime = 0;
}


Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::addRecord(const std::string& name, const float value) {
    records[name].addRecord(value);
}
void Profiler::reset(const std::string& name, const std::string& unit) {
    records[name] = ProfilerRecord();
    records[name].unit = unit;
}

std::vector<std::pair<std::string, ProfilerRecord>> Profiler::getAllRecords() const {
    std::vector<std::pair<std::string, ProfilerRecord>> profilerData(records.begin(), records.end());
    std::sort(profilerData.begin(), profilerData.end(),
        [](const std::pair<std::string, ProfilerRecord>& a, const std::pair<std::string, ProfilerRecord>& b) {
            return a.first < b.first;
        });
    return profilerData;
}

void Profiler::startTimer(const std::string& name) {
    records[name].startTimer();
}

void Profiler::endTimer(const std::string& name) {
    records[name].endTimer();
}
