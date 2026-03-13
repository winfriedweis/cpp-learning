#include "AnimationEngine.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// ─── Static data ──────────────────────────────────────────────────────────────

const std::vector<std::string> AnimationEngine::nemokAsciiArt_ = {
    "███╗   ██╗███████╗███╗   ███╗ ██████╗ ██╗  ██╗",
    "████╗  ██║██╔════╝████╗ ████║██╔═══██╗██║ ██╔╝",
    "██╔██╗ ██║█████╗  ██╔████╔██║██║   ██║█████╔╝ ",
    "██║╚██╗██║██╔══╝  ██║╚██╔╝██║██║   ██║██╔═██╗ ",
    "██║ ╚████║███████╗██║ ╚═╝ ██║╚██████╔╝██║  ██╗",
    "╚═╝  ╚═══╝╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝",
};

const std::vector<std::string> AnimationEngine::morpheusAsciiArt_ = {
    "      ___________",
    "     |           |",
    "     |  @     @  |",
    "     |     ^     |",
    "     |   (   )   |",
    "     |___________|",
    "        /     \\",
    "       /  | |  \\",
    "      /   | |   \\",
};

// ─── Helpers ──────────────────────────────────────────────────────────────────

void AnimationEngine::clearScreen() {
    std::cout << "\x1B[2J\x1B[H";
}

std::string AnimationEngine::getRandomMatrixChar() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(0, 9);
    return std::to_string(dist(gen));
}

// ─── Animations ───────────────────────────────────────────────────────────────

// ─── Matrix rain helpers ──────────────────────────────────────────────────────

// Einzelnes Regen-Zeichen an Position (x, y)
static std::string rainCharAt(int x, int y, const std::vector<int>& col_pos) {
    if (col_pos[x] == y)
        return std::string("\x1B[1;97m") + static_cast<char>('0' + (rand() % 10)) + "\x1B[0m";
    if (col_pos[x] - 1 == y)
        return std::string("\x1B[1;92m") + static_cast<char>('0' + (rand() % 10)) + "\x1B[0m";
    if (col_pos[x] > y && col_pos[x] - 20 < y)
        return std::string("\x1B[32m") + static_cast<char>('0' + (rand() % 10)) + "\x1B[0m";
    return " ";
}

// Eine Zeile der zentrierten Menü-Box (row 0-6)
static std::string boxRowStr(int row, int innerW,
                             const std::string& msg, float prog) {
    auto rep = [](const char* ch, int n) {
        std::string s; for (int i = 0; i < n; i++) s += ch; return s;
    };
    auto center = [&](const std::string& s, int w) {
        int lp = std::max(0, (w - (int)s.size()) / 2);
        int rp = std::max(0, w - lp - (int)s.size());
        return std::string(lp, ' ') + s + std::string(rp, ' ');
    };
    const char* G = "\x1B[1;32m";   // border: bright green
    const char* T = "\x1B[1;92m";   // title:  bright lime
    const char* C = "\x1B[1;96m";   // text:   cyan
    const char* R = "\x1B[0m";

    switch (row) {
        case 0: return std::string(G) + "╔" + rep("═", innerW) + "╗" + R;
        case 1: return std::string(G) + "║" + R + T
                       + center("N  E  M  O  K", innerW) + R + G + "║" + R;
        case 2: return std::string(G) + "║" + R
                       + center("Algorithm Trainer", innerW) + G + "║" + R;
        case 3: return std::string(G) + "╠" + rep("═", innerW) + "╣" + R;
        case 4: {
            std::string m = msg.empty() ? "Initialisierung..." : msg;
            if ((int)m.size() > innerW - 2) m = m.substr(0, innerW - 2);
            int rp = std::max(0, innerW - 1 - (int)m.size());
            return std::string(G) + "║" + R + " " + C + m + R
                   + std::string(rp, ' ') + G + "║" + R;
        }
        case 5: {
            int barW   = innerW - 4;
            int filled = static_cast<int>(prog * barW);
            std::string bar = "\x1B[36m[\x1B[0m";
            bar += "\x1B[1;32m" + std::string(filled, '=');
            if (filled < barW) {
                bar += "\x1B[1;97m>";
                bar += "\x1B[32m" + std::string(barW - filled - 1, ' ');
            }
            bar += "\x1B[36m]\x1B[0m";
            return std::string(G) + "║" + R + " " + bar + " " + G + "║" + R;
        }
        case 6: return std::string(G) + "╚" + rep("═", innerW) + "╝" + R;
        default: return "";
    }
}

// ─── Matrix rain ──────────────────────────────────────────────────────────────

void AnimationEngine::playMatrixRain(int duration_ms, const std::string& loaderMessage) {
    struct winsize ws{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    const int width  = (ws.ws_col > 0 ? ws.ws_col : 80) - 1;
    const int height = (ws.ws_row > 0 ? ws.ws_row : 24) - 1;  // volle Höhe
    const int frames = duration_ms / 50;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> speedDist(1, 2);  // dichter: 1-2 statt 1-3

    std::vector<int> col_pos(width);
    std::vector<int> col_spd(width);
    for (int i = 0; i < width; i++) {
        col_pos[i] = -std::uniform_int_distribution<>(0, height)(gen);
        col_spd[i] = speedDist(gen);
    }

    // Zentrierte Menü-Box im Regen
    const int boxW    = 44;
    const int boxH    = 7;
    const int boxLeft = std::max(0, (width - boxW) / 2);
    const int boxTop  = (height - boxH) / 2;
    const int boxBot  = boxTop + boxH - 1;
    const int innerW  = boxW - 2;

    std::cout << "\x1B[2J\x1B[?25l" << std::flush;

    for (int frame = 0; frame < frames; frame++) {
        const float progress = static_cast<float>(frame) / frames;

        std::string buf;
        buf.reserve(static_cast<size_t>(width * height * 12));
        buf += "\x1B[?25l\x1B[H";

        for (int y = 0; y < height; y++) {
            if (y >= boxTop && y <= boxBot) {
                // Linker Regen
                for (int x = 0; x < boxLeft; x++)
                    buf += rainCharAt(x, y, col_pos);
                // Box-Inhalt
                buf += boxRowStr(y - boxTop, innerW, loaderMessage, progress);
                // Rechter Regen
                for (int x = boxLeft + boxW; x < width; x++)
                    buf += rainCharAt(x, y, col_pos);
            } else {
                // Volle Regen-Zeile
                for (int x = 0; x < width; x++)
                    buf += rainCharAt(x, y, col_pos);
            }
            buf += '\n';
        }

        std::cout << buf;
        std::cout.flush();

        for (int i = 0; i < width; i++) {
            if (frame % col_spd[i] == 0) {
                col_pos[i]++;
                if (col_pos[i] > height + 10)
                    col_pos[i] = -std::uniform_int_distribution<>(0, 10)(gen);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "\x1B[?25h\x1B[0m";
}

void AnimationEngine::playNeoIntro() {
    for (int step = 0; step <= 10; step++) {
        std::vector<Element> elements;

        Color c = (step < 4) ? Color::Green : Color::GreenLight;
        for (const auto& line : nemokAsciiArt_) {
            elements.push_back(text(line) | ftxui::color(c) | bold | center);
        }
        elements.push_back(text(""));
        elements.push_back(
            text("\"There is no spoon... only algorithms\"") |
            color(Color::Cyan) | italic | center
        );

        auto document = vbox(elements);
        auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
        Render(screen, document);

        clearScreen();
        screen.Print();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void AnimationEngine::playGlitchEffect(const std::string& input) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, static_cast<int>(input.length()) - 1);

    for (int frame = 0; frame < 15; frame++) {
        std::string glitched = input;

        if (frame < 10) {
            for (int i = 0; i < 5; i++) {
                int pos = posDist(gen);
                glitched[static_cast<size_t>(pos)] = static_cast<char>('0' + (rand() % 10));
            }
        }

        Color textColor = (frame % 2 == 0) ? Color::Green : Color::Cyan;
        auto document = vbox({
            text(""),
            text(glitched) | color(textColor) | bold | center
        });

        auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
        Render(screen, document);

        clearScreen();
        screen.Print();

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void AnimationEngine::playLoadingBar(const std::string& message, int duration_ms) {
    const int steps = 20;
    const int delay = duration_ms / steps;

    for (int i = 0; i <= steps; i++) {
        float progress = static_cast<float>(i) / steps;

        auto document = vbox({
            text(message) | bold | color(Color::Cyan),
            separator(),
            gauge(progress) | color(Color::Green)
        }) | border | color(Color::White);

        auto screen = Screen::Create(Dimension::Fixed(60), Dimension::Fixed(5));
        Render(screen, document);

        clearScreen();
        screen.Print();

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void AnimationEngine::playMorpheusReveal() {
    std::vector<Element> elements;

    elements.push_back(text(""));
    for (const auto& line : morpheusAsciiArt_) {
        elements.push_back(text(line) | color(Color::Cyan) | center);
    }
    elements.push_back(text(""));
    elements.push_back(
        text("\"I can only show you the door.\"") |
        italic | color(Color::Yellow) | center
    );
    elements.push_back(
        text("\"You're the one who has to walk through it.\"") |
        italic | color(Color::Yellow) | center
    );
    elements.push_back(text(""));

    auto document = vbox(elements) | border | color(Color::Cyan);
    auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
    Render(screen, document);

    clearScreen();
    screen.Print();

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}

// ─── FTXUI Element builders ───────────────────────────────────────────────────

Element AnimationEngine::createFancyHeader(const std::string& title) {
    const int width = 58;
    int padding = (width - static_cast<int>(title.length())) / 2;
    if (padding < 0) padding = 0;
    int rightPad = width - padding - static_cast<int>(title.length());
    if (rightPad < 0) rightPad = 0;

    std::string bar(width, '=');
    return vbox({
        text("+" + bar + "+") | color(Color::Cyan) | bold,
        text("|" + std::string(padding, ' ') +
             title +
             std::string(rightPad, ' ') +
             "|") | color(Color::Yellow) | bold,
        text("+" + bar + "+") | color(Color::Cyan) | bold,
    }) | center;
}

Element AnimationEngine::createNeoHeader() {
    std::vector<Element> elements;

    for (const auto& line : nemokAsciiArt_) {
        elements.push_back(text(line) | bold | color(Color::GreenLight) | center);
    }
    elements.push_back(text(""));
    elements.push_back(
        text("\"There is no spoon... only algorithms\"") |
        italic | color(Color::Cyan) | center
    );

    return vbox(elements);
}
