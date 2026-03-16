#include "TwoSum.h"
#include <unordered_map>

std::vector<int> TwoSum::solve(const std::vector<int>& nums, int target) const {
    std::unordered_map<int, int> seen;

    for (int i = 0; i < std::ssize(nums); ++i) {
        int complement = target - nums[i];

        if (seen.contains(complement)) {
            return {seen[complement], i};
        }

        seen[nums[i]] = i;
    }

    return {};
}
