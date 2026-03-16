#include "MenuRenderer.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <iostream>

using namespace ftxui;

MenuRenderer::MenuRenderer() : selected_(0) {
    entries_ = {
        "  Two Sum  (#1)",
        "  Add Two Numbers  (#2)",
        "  RayCast - Game",
        "  Beenden",
    };
}

void MenuRenderer::setEntries(const std::vector<std::string>& entries) {
    entries_ = entries;
}

void MenuRenderer::playIntroSequence() {
    // Regen + Ladebalken gleichzeitig — dann direkt ins Menü
    animation_.playMatrixRain(3000, "Booting Nemok...");
}

// Build the renderer. `animation_` is a member so the lambda capture is safe.
Component MenuRenderer::buildRenderer(Component menu) {
    return Renderer(menu, [this, menu] {
        return vbox({
            text(""),
            animation_.createNeoHeader(),
            separator(),
            text("  ALGORITHMEN") | bold | color(Color::Yellow) | center,
            separator(),
            menu->Render() | center,
            separator(),
            text("[↑/↓] navigieren  |  [Enter] auswählen") | dim | center,
        }) | border | color(Color::GreenLight);
    });
}

// Key fix: ONE ScreenInteractive instance owns the event loop.
// The ExitLoopClosure is bound to the same instance that runs Loop().
void MenuRenderer::showMenu(SelectionCallback callback) {
    std::cout << "\x1B[2J\x1B[H" << std::flush;  // nach Intro: sauber von oben
    auto screen = ScreenInteractive::TerminalOutput();

    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();

    auto menu     = Menu(&entries_, &selected_, option);
    auto renderer = buildRenderer(menu);

    while (true) {
        screen.Loop(renderer);

        // Last entry is always "Beenden"
        if (selected_ == static_cast<int>(entries_.size()) - 1) {
            std::cout << "\n> Nemok signing off... disconnecting from the Matrix.\n";
            break;
        }

        callback(selected_);

        std::cout << "\n[Press ENTER to return to the Matrix...]\n";
        std::string dummy;
        std::getline(std::cin, dummy);
    }
}
