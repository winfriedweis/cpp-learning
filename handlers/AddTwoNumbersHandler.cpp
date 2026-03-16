#include "AddTwoNumbersHandler.h"
#include "ui/ProblemPresenter.h"
#include "ui/AnimationEngine.h"
#include "algorithms/AddTwoNumbers.h"
#include <iostream>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

namespace {

// ── Helpers ───────────────────────────────────────────────────────────────────

bool isValidNumberStr(const std::string& s) {
    return !s.empty() &&
           std::all_of(s.begin(), s.end(), [](char c) { return std::isdigit(c) != 0; });
}

// "342" → [2,4,3]  (reversed digits, LeetCode linked-list format)
std::vector<int> toReversedDigits(const std::string& s) {
    std::vector<int> digits;
    digits.reserve(s.size());
    for (auto it = s.rbegin(); it != s.rend(); ++it)
        digits.push_back(*it - '0');
    return digits;
}

// ── Input form ────────────────────────────────────────────────────────────────

struct Input {
    std::string num1{"342"};
    std::string num2{"465"};
};

std::optional<Input> promptInput() {
    std::string num1Str = "342";
    std::string num2Str = "465";
    std::string errorMsg;
    std::optional<Input> confirmed;

    auto screen = ScreenInteractive::TerminalOutput();
    auto input1 = ftxui::Input(&num1Str, "z.B. 342");
    auto input2 = ftxui::Input(&num2Str, "z.B. 465");

    auto submitBtn = Button("[ Berechnen ]", [&] {
        if (!isValidNumberStr(num1Str) || !isValidNumberStr(num2Str)) {
            errorMsg = "Ungültig — nur positive ganze Zahlen ohne Leerzeichen, z.B. 342";
            return;
        }
        errorMsg.clear();
        confirmed = Input{num1Str, num2Str};
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
        if (event == Event::Escape) { screen.ExitLoopClosure()(); return true; }
        return false;
    });

    std::cout << "\x1B[2J\x1B[H" << std::flush;
    screen.Loop(withEsc);
    return confirmed;
}

// ── Formatting ────────────────────────────────────────────────────────────────

Element formatWalkthrough(
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

    int carry = 0;
    for (size_t pos = 0; pos < resultDigits.size(); ++pos) {
        const int d1  = (pos < digits1.size()) ? digits1[pos] : 0;
        const int d2  = (pos < digits2.size()) ? digits2[pos] : 0;
        const int sum = d1 + d2 + carry;
        carry = sum / 10;

        const std::string line = "  Pos " + std::to_string(pos + 1) + ":  "
            + std::to_string(d1) + " + " + std::to_string(d2)
            + " + carry(" + std::to_string(sum - d1 - d2) + ")"
            + " = " + std::to_string(sum);
        const std::string outcome = "  →  write " + std::to_string(sum % 10)
            + ",  carry = " + std::to_string(carry);

        steps.push_back(hbox({
            text(line)    | color(Color::White),
            text(outcome) | color(carry > 0 ? Color::Yellow : Color::GreenLight),
        }));
    }
    return vbox(steps);
}

// ── Presentation ─────────────────────────────────────────────────────────────

void present(const std::vector<int>& digits1,
             const std::vector<int>& digits2,
             const std::vector<int>& resultDigits)
{
    AnimationEngine animation;
    animation.playLoadingBar("Linking nodes...", 1000);
    std::cout << "\x1B[2J\x1B[H";

    std::vector<Element> doc;
    doc.push_back(animation.createFancyHeader("Add Two Numbers  ·  LeetCode #2"));
    doc.push_back(text(""));

    doc.push_back(text("PROBLEM") | bold | color(Color::Cyan));
    doc.push_back(hbox({text("  "),
        text("Two numbers are stored digit-by-digit in reverse order in linked") | color(Color::White)}));
    doc.push_back(hbox({text("  "),
        text("lists. Add them and return the sum as a linked list.") | color(Color::White)}));
    doc.push_back(text(""));

    doc.push_back(text("INPUT") | bold | color(Color::Cyan));
    doc.push_back(ProblemPresenter::formatLinkedList(digits1, "L1"));
    doc.push_back(ProblemPresenter::formatLinkedList(digits2, "L2"));
    doc.push_back(text(""));

    doc.push_back(text("HOW IT WORKS  (digit-by-digit with carry)") | bold | color(Color::Cyan));
    doc.push_back(formatWalkthrough(digits1, digits2, resultDigits));
    doc.push_back(text(""));

    doc.push_back(text("RESULT") | bold | color(Color::GreenLight));
    doc.push_back(ProblemPresenter::formatLinkedList(resultDigits, "Sum"));
    doc.push_back(hbox({
        text("  Check : ") | color(Color::Magenta),
        text(ProblemPresenter::digitsToNumberStr(digits1)) | color(Color::Yellow),
        text(" + ")        | color(Color::White),
        text(ProblemPresenter::digitsToNumberStr(digits2)) | color(Color::Yellow),
        text(" = ")        | color(Color::White),
        text(ProblemPresenter::digitsToNumberStr(resultDigits)) | bold | color(Color::GreenLight),
        text("  ✓")        | bold | color(Color::GreenLight),
    }));
    doc.push_back(text(""));

    doc.push_back(ProblemPresenter::formatComplexity(
        AddTwoNumbers::getTimeComplexity(), AddTwoNumbers::getSpaceComplexity(),
        "one pass, length = max(m, n) + possible extra carry node",
        "result list has at most max(m, n) + 1 nodes"));
    doc.push_back(separator());

    doc.push_back(hbox({
        text("  Morpheus: ") | color(Color::Cyan),
        text("\"One node at a time... the carry is the key.\"")
            | italic | color(Color::Yellow),
    }));

    std::cout << "\n";
    ProblemPresenter::printToConsole(vbox(doc));
    std::cout << "\n";
}

} // namespace

// ── Public entry point ────────────────────────────────────────────────────────

void handleAddTwoNumbers() {
    const auto input = promptInput();
    if (!input) return;

    const std::vector<int> digits1 = toReversedDigits(input->num1);
    const std::vector<int> digits2 = toReversedDigits(input->num2);

    AddTwoNumbers algorithm;
    ListNode* l1  = AddTwoNumbers::fromVector(digits1);
    ListNode* l2  = AddTwoNumbers::fromVector(digits2);
    ListNode* res = algorithm.solve(l1, l2);
    const auto resultDigits = AddTwoNumbers::toVector(res);

    AddTwoNumbers::freeList(l1);
    AddTwoNumbers::freeList(l2);
    AddTwoNumbers::freeList(res);

    present(digits1, digits2, resultDigits);
}
