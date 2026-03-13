#include "TwoSum.h"
#include <unordered_map>

std::vector<int> TwoSum::solve(const std::vector<int>& nums, int target) const {
    std::unordered_map<int, int> seen;

    for (size_t i = 0; i < nums.size(); i++) {
        int complement = target - nums[i];

        if (seen.count(complement)) {
            return {seen[complement], static_cast<int>(i)};
        }

        seen[nums[i]] = static_cast<int>(i);
    }

    return {};
}
