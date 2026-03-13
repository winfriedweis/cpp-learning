#pragma once
#include <string>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

// Handles all visual animations and effects (Matrix/Neo theme).
// Pure UI — no business logic.
class AnimationEngine {
public:
    void playMatrixRain(int duration_ms = 3000, const std::string& loaderMessage = "");
    void playNeoIntro();
    void playGlitchEffect(const std::string& text);
    void playLoadingBar(const std::string& message, int duration_ms = 2000);
    void playMorpheusReveal();

    ftxui::Element createFancyHeader(const std::string& title);
    ftxui::Element createNeoHeader();

private:
    void clearScreen();
    std::string getRandomMatrixChar();

    static const std::vector<std::string> nemokAsciiArt_;
    static const std::vector<std::string> morpheusAsciiArt_;
};
