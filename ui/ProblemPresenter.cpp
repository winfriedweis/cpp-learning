#include "ProblemPresenter.h"
#include "AnimationEngine.h"
#include <iostream>
#include <unordered_map>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// ─── Helpers ──────────────────────────────────────────────────────────────────

void ProblemPresenter::printToConsole(Element element) {
    auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(element));
    Render(screen, element);
    screen.Print();
}

// ─── Public ───────────────────────────────────────────────────────────────────

void ProblemPresenter::presentTwoSum(
    const std::vector<int>& nums,
    int target,
    const std::vector<int>& result,
    const std::string& algorithmName,
    const std::string& timeComplexity,
    const std::string& spaceComplexity
) {
    AnimationEngine animation;
    animation.playLoadingBar("Decrypting matrix data...", 1000);
    std::cout << "\x1B[2J\x1B[H";

    std::vector<Element> doc;

    // ── Header ────────────────────────────────────────────────────────────────
    doc.push_back(animation.createFancyHeader(algorithmName + "  ·  LeetCode #1"));
    doc.push_back(text(""));

    // ── Problem description ───────────────────────────────────────────────────
    doc.push_back(text("PROBLEM") | bold | color(Color::Cyan));
    doc.push_back(hbox({
        text("  "),
        text("Given an integer array and a target, return the indices") |
            color(Color::White),
    }));
    doc.push_back(hbox({
        text("  "),
        text("of the two numbers that add up to the target.") |
            color(Color::White),
    }));
    doc.push_back(text(""));

    // ── Input ─────────────────────────────────────────────────────────────────
    doc.push_back(text("INPUT") | bold | color(Color::Cyan));
    doc.push_back(formatArray(nums, "nums  "));
    doc.push_back(hbox({
        text("  target = ") | color(Color::Magenta),
        text(std::to_string(target)) | bold | color(Color::Yellow),
    }));
    doc.push_back(text(""));

    // ── Walkthrough ───────────────────────────────────────────────────────────
    doc.push_back(text("HOW IT WORKS  (Hash Map approach)") | bold | color(Color::Cyan));
    doc.push_back(formatWalkthrough(nums, target, result));
    doc.push_back(text(""));

    // ── Solution ──────────────────────────────────────────────────────────────
    if (!result.empty()) {
        doc.push_back(text("SOLUTION") | bold | color(Color::GreenLight));
        doc.push_back(formatResult(result, nums, target));
    } else {
        doc.push_back(text("NO SOLUTION EXISTS") | bold | color(Color::Red));
    }
    doc.push_back(text(""));

    // ── Complexity ────────────────────────────────────────────────────────────
    doc.push_back(formatComplexity(timeComplexity, spaceComplexity));
    doc.push_back(separator());

    // ── Morpheus quote ────────────────────────────────────────────────────────
    std::string quote = result.empty()
        ? "\"Not all questions have answers, Neo.\""
        : "\"You took the red pill... the indices are ["
          + std::to_string(result[0]) + ", "
          + std::to_string(result[1]) + "]\"";

    doc.push_back(hbox({
        text("  Morpheus: ") | color(Color::Cyan),
        text(quote)         | italic | color(Color::Yellow),
    }));

    std::cout << "\n";
    printToConsole(vbox(doc));
    std::cout << "\n";
}

// ─── Element builders ─────────────────────────────────────────────────────────

// Visualises the hash map approach step by step with the actual input.
Element ProblemPresenter::formatWalkthrough(
    const std::vector<int>& nums,
    int target,
    const std::vector<int>& result
) {
    std::vector<Element> steps;
    steps.push_back(hbox({
        text("  "),
        text("For each num: is (target − num) already in the map?") |
            color(Color::White),
    }));
    steps.push_back(text(""));

    std::unordered_map<int, int> seen;
    for (size_t i = 0; i < nums.size(); i++) {
        int num  = nums[i];
        int need = target - num;

        std::string prefix = "  i=" + std::to_string(i)
                           + "  num=" + std::to_string(num)
                           + "  need=" + std::to_string(need);

        if (seen.count(need)) {
            steps.push_back(hbox({
                text(prefix)                                          | color(Color::White),
                text("  → FOUND at index " + std::to_string(seen[need]) + "!")
                    | bold | color(Color::GreenLight),
            }));
            break;
        } else {
            steps.push_back(hbox({
                text(prefix)                                                  | color(Color::White),
                text("  → store " + std::to_string(num) + ":" + std::to_string(i))
                    | color(Color::Green),
            }));
            seen[num] = static_cast<int>(i);
        }
    }

    return vbox(steps);
}

Element ProblemPresenter::formatArray(
    const std::vector<int>& arr,
    const std::string& label
) {
    std::vector<Element> elems;
    elems.push_back(text("  "));
    elems.push_back(text(label + "= ") | color(Color::Magenta));
    elems.push_back(text("[ ")         | color(Color::Cyan));

    for (size_t i = 0; i < arr.size(); i++) {
        elems.push_back(text(std::to_string(arr[i])) | color(Color::Yellow));
        if (i < arr.size() - 1) {
            elems.push_back(text(", ") | color(Color::Cyan));
        }
    }

    elems.push_back(text(" ]") | color(Color::Cyan));
    return hbox(elems);
}

Element ProblemPresenter::formatResult(
    const std::vector<int>& result,
    const std::vector<int>& nums,
    int target
) {
    return vbox({
        hbox({
            text("  indices = ") | color(Color::Magenta),
            text("[")            | color(Color::Cyan),
            text(std::to_string(result[0])) | bold | color(Color::Yellow),
            text(", ")           | color(Color::Cyan),
            text(std::to_string(result[1])) | bold | color(Color::Yellow),
            text("]")            | color(Color::Cyan),
        }),
        hbox({
            text("  check   = ") | color(Color::Magenta),
            text(std::to_string(nums[result[0]])) | color(Color::Yellow),
            text(" + ")          | color(Color::White),
            text(std::to_string(nums[result[1]])) | color(Color::Yellow),
            text(" = ")          | color(Color::White),
            text(std::to_string(target)) | bold | color(Color::GreenLight),
            text("  ✓")          | bold | color(Color::GreenLight),
        }),
    });
}

Element ProblemPresenter::formatComplexity(
    const std::string& timeComplexity,
    const std::string& spaceComplexity
) {
    return vbox({
        text("COMPLEXITY") | bold | color(Color::Cyan),
        hbox({
            text("  Time:  ")  | color(Color::White),
            text(timeComplexity) | bold | color(Color::GreenLight),
            text("  — one pass through the array") | dim | color(Color::Green),
        }),
        hbox({
            text("  Space: ")  | color(Color::White),
            text(spaceComplexity) | bold | color(Color::GreenLight),
            text("  — hash map holds at most n elements") | dim | color(Color::Green),
        }),
    });
}
