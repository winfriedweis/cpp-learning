#pragma once
#include <vector>

// Given an array of integers and a target, return the indices
// of the two numbers that add up to the target.
// Time: O(n)  |  Space: O(n)
class TwoSum {
public:
    [[nodiscard]] std::vector<int> solve(const std::vector<int>& nums, int target) const;

    static const char* getName()           { return "Two Sum"; }
    static const char* getTimeComplexity() { return "O(n)"; }
    static const char* getSpaceComplexity(){ return "O(n)"; }
};
