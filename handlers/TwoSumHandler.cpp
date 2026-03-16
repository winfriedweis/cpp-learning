#include "TwoSumHandler.h"
#include "ui/ProblemPresenter.h"
#include "ui/AnimationEngine.h"
#include "algorithms/TwoSum.h"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// ── Input parsing ─────────────────────────────────────────────────────────────

namespace {

struct TwoSumInput {
    std::vector<int> nums;
    int target{};
};

std::optional<TwoSumInput> parseTwoSumInput(
    const std::string& numsStr, const std::string& targetStr)
{
    TwoSumInput result;
    try { result.target = std::stoi(targetStr); }
    catch (...) { return std::nullopt; }

    std::istringstream ss(numsStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        const auto first = token.find_first_not_of(" \t");
        const auto last  = token.find_last_not_of(" \t");
        if (first == std::string::npos) continue;
        try { result.nums.push_back(std::stoi(token.substr(first, last - first + 1))); }
        catch (...) { return std::nullopt; }
    }
    if (result.nums.size() < 2) return std::nullopt;
    return result;
}

// ── Input form ────────────────────────────────────────────────────────────────

std::optional<TwoSumInput> promptInput() {
    std::string numsStr   = "2, 7, 11, 15";
    std::string targetStr = "9";
    std::string errorMsg;
    std::optional<TwoSumInput> confirmed;

    auto screen = ScreenInteractive::TerminalOutput();

    auto numsInput   = Input(&numsStr,   "z.B. 2, 7, 11, 15");
    auto targetInput = Input(&targetStr, "z.B. 9");

    auto submitBtn = Button("[ Berechnen ]", [&] {
        auto parsed = parseTwoSumInput(numsStr, targetStr);
        if (!parsed) {
            errorMsg = "Ungültig — kommagetrennte ganze Zahlen (min. 2) und Ziel erforderlich.";
            return;
        }
        errorMsg.clear();
        confirmed = std::move(parsed);
        screen.ExitLoopClosure()();
    }, ButtonOption::Simple());

    auto container = Container::Vertical({numsInput, targetInput, submitBtn});

    auto renderer = Renderer(container, [&] {
        std::vector<Element> rows = {
            text(""),
            text("  TWO SUM  ·  Eigene Eingabe") | bold | color(Color::Yellow) | center,
            separator(),
            text(""),
            hbox({text("  Array  : ") | color(Color::Cyan), numsInput->Render() | flex}),
            hbox({text("  Target : ") | color(Color::Cyan), targetInput->Render() | flex}),
            text(""),
            submitBtn->Render() | center,
            text(""),
            separator(),
        };
        if (!errorMsg.empty())
            rows.push_back(text("  " + errorMsg) | color(Color::Red));
        rows.push_back(
            text("  [Tab] Feld wechseln   [Enter] Berechnen   [Esc] Abbrechen")
                | dim | color(Color::Green) | center);
        rows.push_back(text(""));
        return vbox(rows) | border | color(Color::GreenLight);
    });

    auto withEsc = CatchEvent(renderer, [&](Event event) -> bool {
        if (event == Event::Escape) { screen.ExitLoopClosure()(); return true; }
        return false;
    });

    std::cout << "\x1B[2J\x1B[H" << std::flush;
    screen.Loop(withEsc);
    return confirmed;
}

// ── Formatting ────────────────────────────────────────────────────────────────

Element formatWalkthrough(
    const std::vector<int>& nums, int target, const std::vector<int>& result)
{
    std::vector<Element> steps;
    steps.push_back(hbox({
        text("  "),
        text("For each num: is (target − num) already in the map?") | color(Color::White),
    }));
    steps.push_back(text(""));

    std::unordered_map<int, int> seen;
    for (int i = 0; i < std::ssize(nums); ++i) {
        int num  = nums[i];
        int need = target - num;
        std::string prefix = "  i=" + std::to_string(i)
                           + "  num=" + std::to_string(num)
                           + "  need=" + std::to_string(need);

        if (seen.contains(need)) {
            steps.push_back(hbox({
                text(prefix) | color(Color::White),
                text("  → FOUND at index " + std::to_string(seen[need]) + "!")
                    | bold | color(Color::GreenLight),
            }));
            break;
        } else {
            steps.push_back(hbox({
                text(prefix) | color(Color::White),
                text("  → store " + std::to_string(num) + ":" + std::to_string(i))
                    | color(Color::Green),
            }));
            seen[num] = i;
        }
    }
    return vbox(steps);
}

Element formatResult(
    const std::vector<int>& result, const std::vector<int>& nums, int target)
{
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

// ── Presentation ─────────────────────────────────────────────────────────────

void present(const TwoSumInput& input, const std::vector<int>& result) {
    AnimationEngine animation;
    animation.playLoadingBar("Decrypting matrix data...", 1000);
    std::cout << "\x1B[2J\x1B[H";

    std::vector<Element> doc;
    doc.push_back(animation.createFancyHeader("Two Sum  ·  LeetCode #1"));
    doc.push_back(text(""));

    doc.push_back(text("PROBLEM") | bold | color(Color::Cyan));
    doc.push_back(hbox({text("  "),
        text("Given an integer array and a target, return the indices") | color(Color::White)}));
    doc.push_back(hbox({text("  "),
        text("of the two numbers that add up to the target.") | color(Color::White)}));
    doc.push_back(text(""));

    doc.push_back(text("INPUT") | bold | color(Color::Cyan));
    doc.push_back(ProblemPresenter::formatArray(input.nums, "nums  "));
    doc.push_back(hbox({
        text("  target = ") | color(Color::Magenta),
        text(std::to_string(input.target)) | bold | color(Color::Yellow),
    }));
    doc.push_back(text(""));

    doc.push_back(text("HOW IT WORKS  (Hash Map approach)") | bold | color(Color::Cyan));
    doc.push_back(formatWalkthrough(input.nums, input.target, result));
    doc.push_back(text(""));

    if (!result.empty()) {
        doc.push_back(text("SOLUTION") | bold | color(Color::GreenLight));
        doc.push_back(formatResult(result, input.nums, input.target));
    } else {
        doc.push_back(text("NO SOLUTION EXISTS") | bold | color(Color::Red));
    }
    doc.push_back(text(""));

    doc.push_back(ProblemPresenter::formatComplexity(
        TwoSum::getTimeComplexity(), TwoSum::getSpaceComplexity(),
        "one pass through the array",
        "hash map holds at most n elements"));
    doc.push_back(separator());

    const std::string quote = result.empty()
        ? "\"Not all questions have answers, Neo.\""
        : "\"You took the red pill... the indices are ["
          + std::to_string(result[0]) + ", " + std::to_string(result[1]) + "]\"";
    doc.push_back(hbox({
        text("  Morpheus: ") | color(Color::Cyan),
        text(quote)          | italic | color(Color::Yellow),
    }));

    std::cout << "\n";
    ProblemPresenter::printToConsole(vbox(doc));
    std::cout << "\n";
}

} // namespace

// ── Public entry point ────────────────────────────────────────────────────────

void handleTwoSum() {
    const auto input = promptInput();
    if (!input) return;

    const TwoSum algorithm;
    const auto result = algorithm.solve(input->nums, input->target);
    present(*input, result);
}
