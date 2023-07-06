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