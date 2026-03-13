#include "ui/MenuRenderer.h"
#include "ui/ProblemPresenter.h"
#include "algorithms/TwoSum.h"
#include "games/RayCaster.h"
#include <iostream>
#include <vector>

// ─── Algorithm handlers ───────────────────────────────────────────────────────

static void handleTwoSum() {
    // Business logic (pure, no UI)
    TwoSum algorithm;
    std::vector<int> nums = {2, 7, 11, 15};
    int target = 9;
    std::vector<int> result = algorithm.solve(nums, target);

    // Presentation (UI layer)
    ProblemPresenter presenter;
    presenter.presentTwoSum(
        nums,
        target,
        result,
        TwoSum::getName(),
        TwoSum::getTimeComplexity(),
        TwoSum::getSpaceComplexity()
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
            case 0: handleTwoSum(); break;
            case 1: handleRayCaster(); break;
            default: break;
        }
    });

    return 0;
}
