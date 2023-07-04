#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

class Counter {
private:
    std::unordered_map<std::string, int> counts;

    Counter() {}

public:
    static Counter& getInstance();

    void increment(const std::string& name);
    void reset(const std::string& name);
    int getCount(const std::string& name) const;
    std::vector<std::pair<std::string, int>> getAllCounters() const;
};

#ifdef ENABLE_REPORTING
#define INC_COUNTER(name) Counter::getInstance().increment(name)
#else
#define INC_COUNTER(name)
#endif

#endif // COUNTER_HPP