#pragma once
#include <functional>
#include <string>
#include <vector>

struct AlgoEntry {
    std::string           name;
    std::function<void()> run;
};

std::vector<AlgoEntry> getAllEntries();
