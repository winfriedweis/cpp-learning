#pragma once
#include <optional>
#include <vector>
#include <string>
#include <ftxui/dom/elements.hpp>

struct TwoSumInput {
    std::vector<int> nums;
    int target{};
};

// User enters two numbers as plain strings, e.g. "342" and "465".
// main.cpp converts them to reversed-digit vectors for the algorithm.
struct AddTwoNumbersInput {
    std::string num1{"342"};
    std::string num2{"465"};
};

// Formats and displays algorithm results in an educational way.
// No animation logic here — delegates to AnimationEngine.
class ProblemPresenter {
public:
    // Interactive input forms — return nullopt if the user cancels (Esc).
    [[nodiscard]] std::optional<TwoSumInput>          promptTwoSumInput();
    [[nodiscard]] std::optional<AddTwoNumbersInput>   promptAddTwoNumbersInput();

    void presentTwoSum(
        const std::vector<int>& nums,
        int target,
        const std::vector<int>& result,
        const std::string& algorithmName,
        const std::string& timeComplexity,
        const std::string& spaceComplexity
    );

    // digits1 / digits2 / resultDigits are all in reverse order (LeetCode format).
    void presentAddTwoNumbers(
        const std::vector<int>& digits1,
        const std::vector<int>& digits2,
        const std::vector<int>& resultDigits,
        const std::string& algorithmName,
        const std::string& timeComplexity,
        const std::string& spaceComplexity
    );

private:
    void printToConsole(ftxui::Element element);

    ftxui::Element formatArray(const std::vector<int>& arr, const std::string& label);
    ftxui::Element formatComplexity(
        const std::string& timeComplexity,
        const std::string& spaceComplexity
    );
    // TwoSum helpers
    ftxui::Element formatResult(
        const std::vector<int>& result,
        const std::vector<int>& nums,
        int target
    );
    ftxui::Element formatWalkthrough(
        const std::vector<int>& nums,
        int target,
        const std::vector<int>& result
    );
    // AddTwoNumbers helpers
    ftxui::Element formatLinkedList(const std::vector<int>& digits, const std::string& label);
    ftxui::Element formatAddWalkthrough(
        const std::vector<int>& digits1,
        const std::vector<int>& digits2,
        const std::vector<int>& resultDigits
    );
};
