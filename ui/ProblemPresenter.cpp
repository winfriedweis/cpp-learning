#include "ProblemPresenter.h"
#include "AnimationEngine.h"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// ─── Input parsing ────────────────────────────────────────────────────────────

static std::optional<TwoSumInput> parseTwoSumInput(
    const std::string& numsStr, const std::string& targetStr)
{
    TwoSumInput result;

    try {
        result.target = std::stoi(targetStr);
    } catch (...) {
        return std::nullopt;
    }

    std::istringstream ss(numsStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        const auto first = token.find_first_not_of(" \t");
        const auto last  = token.find_last_not_of(" \t");
        if (first == std::string::npos) continue;
        try {
            result.nums.push_back(std::stoi(token.substr(first, last - first + 1)));
        } catch (...) {
            return std::nullopt;
        }
    }

    if (result.nums.size() < 2) return std::nullopt;
    return result;
}

// ─── Interactive input form ───────────────────────────────────────────────────

std::optional<TwoSumInput> ProblemPresenter::promptTwoSumInput() {
    using namespace ftxui;

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
        if (event == Event::Escape) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    std::cout << "\x1B[2J\x1B[H" << std::flush;
    screen.Loop(withEsc);
    return confirmed;
}

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

// ═══════════════════════════════════════════════════════════════════════════════
// ADD TWO NUMBERS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Input validation ─────────────────────────────────────────────────────────

static bool isValidNumberStr(const std::string& s) {
    return !s.empty() &&
           std::all_of(s.begin(), s.end(), [](char c) { return std::isdigit(c) != 0; });
}

// ─── Reversed-digit vector → readable number string ───────────────────────────

static std::string digitsToNumberStr(const std::vector<int>& digits) {
    // digits are in reverse order: [2,4,3] → "342"
    std::string s;
    s.reserve(digits.size());
    for (auto it = digits.rbegin(); it != digits.rend(); ++it)
        s += std::to_string(*it);
    return s.empty() ? "0" : s;
}

// ─── Interactive input form ───────────────────────────────────────────────────

std::optional<AddTwoNumbersInput> ProblemPresenter::promptAddTwoNumbersInput() {
    std::string num1Str = "342";
    std::string num2Str = "465";
    std::string errorMsg;
    std::optional<AddTwoNumbersInput> confirmed;

    auto screen = ScreenInteractive::TerminalOutput();

    auto input1 = Input(&num1Str, "z.B. 342");
    auto input2 = Input(&num2Str, "z.B. 465");

    auto submitBtn = Button("[ Berechnen ]", [&] {
        if (!isValidNumberStr(num1Str) || !isValidNumberStr(num2Str)) {
            errorMsg = "Ungültig — nur positive ganze Zahlen ohne Leerzeichen, z.B. 342";
            return;
        }
        errorMsg.clear();
        confirmed = AddTwoNumbersInput{num1Str, num2Str};
        screen.ExitLoopClosure()();
    }, ButtonOption::Simple());

    auto container = Container::Vertical({input1, input2, submitBtn});

    auto renderer = Renderer(container, [&] {
        std::vector<Element> rows = {
            text(""),
            text("  ADD TWO NUMBERS  ·  LeetCode #2  ·  Eigene Eingabe")
                | bold | color(Color::Yellow) | center,
            separator(),
            text(""),
            text("  Gib zwei Zahlen ein — sie werden als verkettete Listen addiert.")
                | dim | color(Color::White),
            text(""),
            hbox({text("  Zahl 1 : ") | color(Color::Cyan), input1->Render() | flex}),
            hbox({text("  Zahl 2 : ") | color(Color::Cyan), input2->Render() | flex}),
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
        if (event == Event::Escape) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    std::cout << "\x1B[2J\x1B[H" << std::flush;
    screen.Loop(withEsc);
    return confirmed;
}

// ─── Present ──────────────────────────────────────────────────────────────────

void ProblemPresenter::presentAddTwoNumbers(
    const std::vector<int>& digits1,
    const std::vector<int>& digits2,
    const std::vector<int>& resultDigits,
    const std::string& algorithmName,
    const std::string& timeComplexity,
    const std::string& spaceComplexity
) {
    AnimationEngine animation;
    animation.playLoadingBar("Linking nodes...", 1000);
    std::cout << "\x1B[2J\x1B[H";

    std::vector<Element> doc;

    // ── Header ────────────────────────────────────────────────────────────────
    doc.push_back(animation.createFancyHeader(algorithmName + "  ·  LeetCode #2"));
    doc.push_back(text(""));

    // ── Problem description ───────────────────────────────────────────────────
    doc.push_back(text("PROBLEM") | bold | color(Color::Cyan));
    doc.push_back(hbox({text("  "),
        text("Two numbers are stored digit-by-digit in reverse order in linked")
            | color(Color::White)}));
    doc.push_back(hbox({text("  "),
        text("lists. Add them and return the sum as a linked list.")
            | color(Color::White)}));
    doc.push_back(text(""));

    // ── Input ─────────────────────────────────────────────────────────────────
    doc.push_back(text("INPUT") | bold | color(Color::Cyan));
    doc.push_back(formatLinkedList(digits1, "L1"));
    doc.push_back(formatLinkedList(digits2, "L2"));
    doc.push_back(text(""));

    // ── Walkthrough ───────────────────────────────────────────────────────────
    doc.push_back(text("HOW IT WORKS  (digit-by-digit with carry)") | bold | color(Color::Cyan));
    doc.push_back(formatAddWalkthrough(digits1, digits2, resultDigits));
    doc.push_back(text(""));

    // ── Result ────────────────────────────────────────────────────────────────
    doc.push_back(text("RESULT") | bold | color(Color::GreenLight));
    doc.push_back(formatLinkedList(resultDigits, "Sum"));
    doc.push_back(hbox({
        text("  Check : ") | color(Color::Magenta),
        text(digitsToNumberStr(digits1)) | color(Color::Yellow),
        text(" + ")        | color(Color::White),
        text(digitsToNumberStr(digits2)) | color(Color::Yellow),
        text(" = ")        | color(Color::White),
        text(digitsToNumberStr(resultDigits)) | bold | color(Color::GreenLight),
        text("  ✓")        | bold | color(Color::GreenLight),
    }));
    doc.push_back(text(""));

    // ── Complexity ────────────────────────────────────────────────────────────
    doc.push_back(text("COMPLEXITY") | bold | color(Color::Cyan));
    doc.push_back(hbox({
        text("  Time:  ") | color(Color::White),
        text(timeComplexity) | bold | color(Color::GreenLight),
        text("  — one pass, length = max(m, n) + possible extra carry node")
            | dim | color(Color::Green),
    }));
    doc.push_back(hbox({
        text("  Space: ") | color(Color::White),
        text(spaceComplexity) | bold | color(Color::GreenLight),
        text("  — result list has at most max(m, n) + 1 nodes")
            | dim | color(Color::Green),
    }));
    doc.push_back(separator());

    // ── Quote ─────────────────────────────────────────────────────────────────
    doc.push_back(hbox({
        text("  Morpheus: ") | color(Color::Cyan),
        text("\"One node at a time... the carry is the key.\"")
            | italic | color(Color::Yellow),
    }));

    std::cout << "\n";
    printToConsole(vbox(doc));
    std::cout << "\n";
}

// ─── Element builders ─────────────────────────────────────────────────────────

Element ProblemPresenter::formatLinkedList(
    const std::vector<int>& digits, const std::string& label)
{
    std::vector<Element> elems;
    elems.push_back(text("  " + label + " : ") | color(Color::Magenta));

    for (size_t i = 0; i < digits.size(); i++) {
        elems.push_back(
            text("[" + std::to_string(digits[i]) + "]") | bold | color(Color::Yellow));
        if (i < digits.size() - 1)
            elems.push_back(text(" → ") | color(Color::Cyan));
    }

    elems.push_back(text("  →  null") | dim | color(Color::Green));
    elems.push_back(text("   (= " + digitsToNumberStr(digits) + ")")
                        | dim | color(Color::White));

    return hbox(elems);
}

Element ProblemPresenter::formatAddWalkthrough(
    const std::vector<int>& digits1,
    const std::vector<int>& digits2,
    const std::vector<int>& resultDigits)
{
    std::vector<Element> steps;
    steps.push_back(hbox({
        text("  "),
        text("At each position: sum = d1 + d2 + carry;  write sum%10,  carry = sum/10")
            | color(Color::White),
    }));
    steps.push_back(text(""));

    const auto len = resultDigits.size();
    int carry = 0;

    for (size_t pos = 0; pos < len; pos++) {
        const int d1 = (pos < digits1.size()) ? digits1[pos] : 0;
        const int d2 = (pos < digits2.size()) ? digits2[pos] : 0;
        const int sum = d1 + d2 + carry;
        const int written = sum % 10;
        carry = sum / 10;

        std::string line = "  Pos " + std::to_string(pos + 1) + ":  "
            + std::to_string(d1) + " + " + std::to_string(d2)
            + " + carry(" + std::to_string(sum - d1 - d2) + ")"
            + " = " + std::to_string(sum);

        std::string outcome = "  →  write " + std::to_string(written)
            + ",  carry = " + std::to_string(carry);

        steps.push_back(hbox({
            text(line)    | color(Color::White),
            text(outcome) | color(carry > 0 ? Color::Yellow : Color::GreenLight),
        }));
    }

    return vbox(steps);
}
