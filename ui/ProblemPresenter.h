#pragma once
#include <vector>
#include <string>
#include <ftxui/dom/elements.hpp>

// Formats and displays algorithm results in an educational way.
// No animation logic here — delegates to AnimationEngine.
class ProblemPresenter {
public:
    void presentTwoSum(
        const std::vector<int>& nums,
        int target,
        const std::vector<int>& result,
        const std::string& algorithmName,
        const std::string& timeComplexity,
        const std::string& spaceComplexity
    );

private:
    void printToConsole(ftxui::Element element);

    ftxui::Element formatArray(const std::vector<int>& arr, const std::string& label);
    ftxui::Element formatResult(
        const std::vector<int>& result,
        const std::vector<int>& nums,
        int target
    );
    ftxui::Element formatComplexity(
        const std::string& timeComplexity,
        const std::string& spaceComplexity
    );
    // Step-by-step walkthrough of the hash map approach
    ftxui::Element formatWalkthrough(
        const std::vector<int>& nums,
        int target,
        const std::vector<int>& result
    );
};
