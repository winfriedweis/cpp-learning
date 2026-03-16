#include "ui/MenuRenderer.h"
#include "handlers/Registry.h"
#include <iostream>

int main() {
    const auto entries = getAllEntries();

    std::vector<std::string> names;
    names.reserve(entries.size() + 1);
    for (const auto& e : entries)
        names.push_back(e.name);
    names.push_back("  Beenden");

    MenuRenderer menu;
    menu.setEntries(names);
    menu.playIntroSequence();

    menu.showMenu([&](int selection) {
        std::cout << "\x1B[2J\x1B[H";
        entries[selection].run();
    });

    return 0;
}
