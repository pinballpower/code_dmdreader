#include "profiler.hpp"

ProfilerRecord::ProfilerRecord()
{
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

Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::addRecord(const std::string& name, const float value) {
    records[name].addRecord(value);
}
void Profiler::reset(const std::string& name) {
    records[name] = ProfilerRecord();
}
