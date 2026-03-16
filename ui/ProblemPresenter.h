#pragma once
#include <string>
#include <vector>
#include <ftxui/dom/elements.hpp>

// Generic display utilities shared across all problem handlers.
// All methods are static — instantiation not required.
class ProblemPresenter {
public:
    static void printToConsole(ftxui::Element element);

    static ftxui::Element formatArray(
        const std::vector<int>& arr, const std::string& label);

    // time/space: e.g. "O(n)". timeNote/spaceNote: short plain-text explanation.
    static ftxui::Element formatComplexity(
        const std::string& time,      const std::string& space,
        const std::string& timeNote,  const std::string& spaceNote);

    // digits in reverse order (LeetCode linked-list format).
    static ftxui::Element formatLinkedList(
        const std::vector<int>& digits, const std::string& label);

    // [2,4,3] → "342"
    static std::string digitsToNumberStr(const std::vector<int>& digits);
};
