#include "ui/MenuRenderer.h"
#include "ui/ProblemPresenter.h"
#include "algorithms/TwoSum.h"
#include "algorithms/AddTwoNumbers.h"
#include "games/RayCaster.h"
#include <algorithm>
#include <iostream>
#include <vector>

// ─── Algorithm handlers ───────────────────────────────────────────────────────

static void handleTwoSum() {
    ProblemPresenter presenter;
    auto input = presenter.promptTwoSumInput();
    if (!input) return;  // user cancelled

    TwoSum algorithm;
    const auto result = algorithm.solve(input->nums, input->target);

    presenter.presentTwoSum(
        input->nums,
        input->target,
        result,
        TwoSum::getName(),
        TwoSum::getTimeComplexity(),
        TwoSum::getSpaceComplexity()
    );
}

static void handleAddTwoNumbers() {
    ProblemPresenter presenter;
    auto input = presenter.promptAddTwoNumbersInput();
    if (!input) return;

    // Convert number string to reversed-digit vector: "342" → [2, 4, 3]
    auto toReversedDigits = [](const std::string& s) {
        std::vector<int> digits;
        digits.reserve(s.size());
        for (auto it = s.rbegin(); it != s.rend(); ++it)
            digits.push_back(*it - '0');
        return digits;
    };

    const auto digits1 = toReversedDigits(input->num1);
    const auto digits2 = toReversedDigits(input->num2);

    AddTwoNumbers algorithm;
    ListNode* l1  = AddTwoNumbers::fromVector(digits1);
    ListNode* l2  = AddTwoNumbers::fromVector(digits2);
    ListNode* res = algorithm.solve(l1, l2);
    const auto resultDigits = AddTwoNumbers::toVector(res);

    AddTwoNumbers::freeList(l1);
    AddTwoNumbers::freeList(l2);
    AddTwoNumbers::freeList(res);

    presenter.presentAddTwoNumbers(
        digits1, digits2, resultDigits,
        AddTwoNumbers::getName(),
        AddTwoNumbers::getTimeComplexity(),
        AddTwoNumbers::getSpaceComplexity()
    );
}

// ─── Game handlers ───────────────────────────────────────────────────────

static void handleRayCaster() {
  RayCaster game;
  game.run();
}

// ─── Entry point ──────────────────────────────────────────────────────────────

int main() {
    MenuRenderer menu;
    menu.playIntroSequence();

    menu.showMenu([](int selection) {
        std::cout << "\x1B[2J\x1B[H";
        switch (selection) {
            case 0: handleTwoSum();         break;
            case 1: handleAddTwoNumbers();  break;
            case 2: handleRayCaster();      break;
            default: break;
        }
    });

    return 0;
}
