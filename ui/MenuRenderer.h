#pragma once
#include "AnimationEngine.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <functional>
#include <vector>
#include <string>

// Handles the interactive menu. Uses FTXUI for rendering.
// AnimationEngine is a member so the renderer lambda never dangles.
class MenuRenderer {
public:
    using SelectionCallback = std::function<void(int)>;

    MenuRenderer();

    void playIntroSequence();
    void showMenu(SelectionCallback callback);
    void setEntries(const std::vector<std::string>& entries);

private:
    std::vector<std::string> entries_;
    int selected_ = 0;
    AnimationEngine animation_;

    ftxui::Component buildRenderer(ftxui::Component menu);
};
