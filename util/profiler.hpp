#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

class ProfilerRecord {
    public:
        float min = std::numeric_limits<float>::max();
        float max = std::numeric_limits<float>::min();
        int count = 0;
        float sum = 0;
        std::string unit = "";

        void addRecord(const float v);
        float avg() const;

        ProfilerRecord();
};

class Profiler {
private:
    std::unordered_map<std::string, ProfilerRecord> records;

    Profiler() {}

public:
    static Profiler& getInstance();

    void addRecord(const std::string& name, const float value);
    void reset(const std::string& name, const std::string& unit);
    ProfilerRecord getData(const std::string& name) const;
    std::vector<std::pair<std::string, ProfilerRecord>> getAllRecords() const;
};

#ifdef ENABLE_PROFILING
#define ADD_PROFILE_VALUE(name,value) Profiler::getInstance().addRecord(name,value)
#define REGISTER_PROFILE(name, unit) Profiler::getInstance().reset(name, unit)
#else
#define ADD_PROFILE_VALUE(name,value)
#define REGISTER_PROFILE(name, unit)
#endif

#endif // COUNTER_HPP