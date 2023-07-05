#include "counter.hpp"

Counter& Counter::getInstance() {
    static Counter instance;
    return instance;
}

void Counter::increment(const std::string& name) {
    counts[name]++;
}
void Counter::reset(const std::string& name) {
    counts[name] = 0;
}

int Counter::getCount(const std::string& name) const {
    if (counts.count(name) > 0) {
        return counts.at(name);
    }
    return 0;
}

std::vector<std::pair<std::string, int>> Counter::getAllCounters() const {
    std::vector<std::pair<std::string, int>> counterList(counts.begin(), counts.end());
    std::sort(counterList.begin(), counterList.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.first < b.first;
        });
    return counterList;
}