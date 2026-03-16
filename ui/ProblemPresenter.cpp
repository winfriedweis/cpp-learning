#include "ProblemPresenter.h"
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>
#include <iostream>
using namespace ftxui;

void ProblemPresenter::printToConsole(Element element) {
    auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(element));
    Render(screen, element);
    screen.Print();
}

ftxui::Element ProblemPresenter::formatArray(
    const std::vector<int>& arr, const std::string& label)
{
    std::vector<Element> elems;
    elems.push_back(text("  "));
    elems.push_back(text(label + "= ") | color(Color::Magenta));
    elems.push_back(text("[ ")         | color(Color::Cyan));
    for (size_t i = 0; i < arr.size(); i++) {
        elems.push_back(text(std::to_string(arr[i])) | color(Color::Yellow));
        if (i < arr.size() - 1)
            elems.push_back(text(", ") | color(Color::Cyan));
    }
    elems.push_back(text(" ]") | color(Color::Cyan));
    return hbox(elems);
}

ftxui::Element ProblemPresenter::formatComplexity(
    const std::string& time,      const std::string& space,
    const std::string& timeNote,  const std::string& spaceNote)
{
    return vbox({
        text("COMPLEXITY") | bold | color(Color::Cyan),
        hbox({
            text("  Time:  ")  | color(Color::White),
            text(time)   | bold | color(Color::GreenLight),
            text("  — " + timeNote) | dim | color(Color::Green),
        }),
        hbox({
            text("  Space: ")  | color(Color::White),
            text(space) | bold | color(Color::GreenLight),
            text("  — " + spaceNote) | dim | color(Color::Green),
        }),
    });
}

std::string ProblemPresenter::digitsToNumberStr(const std::vector<int>& digits) {
    std::string s;
    s.reserve(digits.size());
    for (auto it = digits.rbegin(); it != digits.rend(); ++it)
        s += std::to_string(*it);
    return s.empty() ? "0" : s;
}

ftxui::Element ProblemPresenter::formatLinkedList(
    const std::vector<int>& digits, const std::string& label)
{
    std::vector<Element> elems;
    elems.push_back(text("  " + label + " : ") | color(Color::Magenta));
    for (size_t i = 0; i < digits.size(); i++) {
        elems.push_back(text("[" + std::to_string(digits[i]) + "]") | bold | color(Color::Yellow));
        if (i < digits.size() - 1)
            elems.push_back(text(" → ") | color(Color::Cyan));
    }
    elems.push_back(text("  →  null") | dim | color(Color::Green));
    elems.push_back(text("   (= " + digitsToNumberStr(digits) + ")") | dim | color(Color::White));
    return hbox(elems);
}
