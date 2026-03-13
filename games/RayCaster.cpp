#include "RayCaster.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// Shade index + Wand-Typ → UTF-8 Zeichen mit ANSI Farbe
static std::string shadeToUTF8(char s, char wtype) {
    if (s == 0) return " ";
    const char* block;
    switch (s) {
        case 4: block = "\xE2\x96\x88"; break;  // █
        case 3: block = "\xE2\x96\x93"; break;  // ▓
        case 2: block = "\xE2\x96\x92"; break;  // ▒
        case 1: block = "\xE2\x96\x91"; break;  // ░
        default: return " ";
    }
    if (wtype == 'f') return std::string("\x1B[32m")    + block + "\x1B[0m"; // Boden: grün
    if (wtype == '>') return std::string("\x1B[1;33m>")          + "\x1B[0m"; // Pfeil gelb
    if (wtype == '<') return std::string("\x1B[1;33m<")          + "\x1B[0m";
    if (wtype == '^') return std::string("\x1B[1;33m^")          + "\x1B[0m";
    if (wtype == 'v') return std::string("\x1B[1;33mv")          + "\x1B[0m";
    if (wtype == 'R') return std::string("\x1B[1;36m")  + block + "\x1B[0m"; // Hase: cyan
    if (wtype == 'E') return std::string("\x1B[1;33m")  + block + "\x1B[0m"; // Exit: gold
    return             std::string("\x1B[37m")           + block + "\x1B[0m"; // Wand: weiß
}

int RayCaster::run()
{
    // ─── Terminal-Größe ───────────────────────────────────────────────────────
    struct winsize ws{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    const int nScreenWidth  = (ws.ws_col > 0 ? ws.ws_col  : 120) - 1;
    const int nScreenHeight = (ws.ws_row > 0 ? ws.ws_row  : 40);

    screen_.assign(nScreenWidth * nScreenHeight, 0);
    wallTypeBuf_.assign(nScreenWidth * nScreenHeight, 0);

    float fPlayerX = 2.0f;
    float fPlayerY = 1.5f;
    float fPlayerA = 0.0f;
    const int   nMapWidth  = 16;
    const int   nMapHeight = 16;
    const float fFOV       = 3.14159f / 4.0f;
    const float fDepth     = 16.0f;

    std::wstring map;
    //           0123456789012345
    map += L"################";  // 0
    map += L"#   #     #    #";  // 1  ← Start (2, 1.5)
    map += L"# # # ### # ## #";  // 2
    map += L"# # # # # #    #";  // 3
    map += L"# # ### # ######";  // 4
    map += L"# #   v # #    #";  // 5  ← Pfeil: nach unten
    map += L"# ### # # # ## #";  // 6
    map += L"#   # #   #  # #";  // 7
    map += L"### # ##### ## #";  // 8
    map += L"#   #   >   #  #";  // 9  ← Pfeil: nach rechts
    map += L"# ##### # ### ##";  // 10
    map += L"# #   R #   #  #";  // 11 ← Hase-Schild (cyan)
    map += L"# # ### ##### ##";  // 12
    map += L"# #     #      #";  // 13
    map += L"#   #####      E";  // 14 ← EXIT (gold)
    map += L"################";  // 15

    // ─── Start-Screen (FTXUI) ─────────────────────────────────────────────────
    {
        auto doc = vbox({
            text(""),
            text(R"( (\(\     )") | center,
            text(R"(  (-.-)   )") | center,
            text(R"(o_(")(")  )") | center,
            text(""),
            text("Follow the rabbit...") | bold | color(Color::GreenLight) | center,
            text(""),
            text("[  Enter  ]") | dim | color(Color::Cyan) | center,
            text(""),
        }) | border | color(Color::Green);

        auto scr = Screen::Create(Dimension::Full(), Dimension::Fit(doc));
        Render(scr, doc);
        std::cout << "\x1B[2J\x1B[H" << std::flush;
        scr.Print();
        std::cout << std::flush;

        tcflush(STDIN_FILENO, TCIFLUSH);  // Eingabepuffer leeren
        std::cin.get();                   // auf Enter warten
    }

    // ─── Raw Mode ─────────────────────────────────────────────────────────────
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN]  = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    const int kHoldFrames = 10;
    int lastW = -100, lastS = -100, lastA = -100, lastD = -100;
    int frame = 0;

    // ─── Game Loop ────────────────────────────────────────────────────────────
    while (true) {
        frame++;

        // Input
        bool kQuit = false;
        char c;
        while (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'w' || c == 'W') lastW = frame;
            if (c == 's' || c == 'S') lastS = frame;
            if (c == 'a' || c == 'A') lastA = frame;
            if (c == 'd' || c == 'D') lastD = frame;
            if (c == 'q' || c == 27)  kQuit = true;
        }

        bool kW = (frame - lastW) <= kHoldFrames;
        bool kS = (frame - lastS) <= kHoldFrames;
        bool kA = (frame - lastA) <= kHoldFrames;
        bool kD = (frame - lastD) <= kHoldFrames;

        if (kQuit) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            std::cout << "\x1B[?25h\x1B[0m\n";
            return 0;
        }

        if (kA) fPlayerA -= 0.05f;
        if (kD) fPlayerA += 0.05f;

        // Bewegung mit Exit-Erkennung
        bool exitReached = false;
        auto tryMove = [&](float dx, float dy) {
            float nx = fPlayerX + dx;
            float ny = fPlayerY + dy;
            if ((int)nx >= 0 && (int)nx < nMapWidth &&
                (int)ny >= 0 && (int)ny < nMapHeight) {
                wchar_t cell = map[(int)ny * nMapWidth + (int)nx];
                if (cell == L'E')  exitReached = true;
                else if (cell == L' ') { fPlayerX = nx; fPlayerY = ny; }
                // alle anderen Chars (#, >, <, ^, v, R) = Wand → nicht bewegen
            }
        };

        if (kW) tryMove( sinf(fPlayerA) * 0.2f,  cosf(fPlayerA) * 0.2f);
        if (kS) tryMove(-sinf(fPlayerA) * 0.2f, -cosf(fPlayerA) * 0.2f);

        if (exitReached) break;

        // ─── Raycasting ───────────────────────────────────────────────────────
        for (int x = 0; x < nScreenWidth; x++) {
            float fRayAngle = (fPlayerA - fFOV / 2.0f) +
                              ((float)x / (float)nScreenWidth) * fFOV;
            float   fDistanceToWall = 0.0f;
            bool    bHitWall = false;
            wchar_t hitCell  = L'#';
            float   fEyeX = sinf(fRayAngle);
            float   fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth) {
                fDistanceToWall += 0.1f;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
                if (nTestX < 0 || nTestX >= nMapWidth ||
                    nTestY < 0 || nTestY >= nMapHeight) {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                } else {
                    wchar_t cell = map[nTestY * nMapWidth + nTestX];
                    if (cell != L' ') {   // alle Nicht-Leerzeichen = Wand (inkl. E, >, R…)
                        bHitWall = true;
                        hitCell  = cell;
                    }
                }
            }

            char nShade = 0;
            if      (fDistanceToWall <= fDepth / 4.0f) nShade = 4;
            else if (fDistanceToWall <  fDepth / 3.0f) nShade = 3;
            else if (fDistanceToWall <  fDepth / 2.0f) nShade = 2;
            else if (fDistanceToWall <  fDepth)         nShade = 1;

            const float fAspect = 0.5f;
            int nCeiling = (int)((float)nScreenHeight / 2.0f -
                                 (float)nScreenHeight * fAspect / fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            char wtype = (hitCell < 128) ? (char)hitCell : '#';

            for (int y = 0; y < nScreenHeight; y++) {
                int idx = y * nScreenWidth + x;
                if (y < nCeiling) {
                    screen_[idx] = 0; wallTypeBuf_[idx] = 0;
                } else if (y > nCeiling && y <= nFloor) {
                    screen_[idx] = nShade; wallTypeBuf_[idx] = wtype;
                } else {
                    float fd = 1.0f - ((float)(y - nFloor) /
                                       (float)(nScreenHeight - nFloor));
                    screen_[idx] = (fd < 0.25f) ? 4 : (fd < 0.5f) ? 3 :
                                   (fd < 0.75f) ? 2 : (fd < 0.9f)  ? 1 : 0;
                    wallTypeBuf_[idx] = 'f';
                }
            }
        }

        // ─── Frame-Output ─────────────────────────────────────────────────────
        std::string out = "\x1B[?25l\x1B[H";
        for (int y = 0; y < nScreenHeight; y++) {
            for (int x = 0; x < nScreenWidth; x++)
                out += shadeToUTF8(screen_[y * nScreenWidth + x],
                                   wallTypeBuf_[y * nScreenWidth + x]);
            out += '\n';
        }

        // ─── Minimap (ANSI absolut, oben rechts) ──────────────────────────────
        if (nScreenWidth > nMapWidth + 4) {
            int mmCol = nScreenWidth - nMapWidth + 1;  // 1-indizierte Startkolonne

            float a = fmodf(fPlayerA, 6.28318f);
            if (a < 0) a += 6.28318f;
            char dirChar = (a < 0.785f || a >= 5.498f) ? 'v'   // Süd
                         : (a < 2.356f)                ? '>'   // Ost
                         : (a < 3.927f)                ? '^'   // Nord
                         :                               '<';  // West

            for (int my = 0; my < nMapHeight; my++) {
                out += "\x1B[" + std::to_string(my + 1) + ";" +
                       std::to_string(mmCol) + "H";
                for (int mx = 0; mx < nMapWidth; mx++) {
                    wchar_t cell = map[my * nMapWidth + mx];
                    if (mx == (int)fPlayerX && my == (int)fPlayerY) {
                        out += std::string("\x1B[1;92m") + dirChar + "\x1B[0m";
                    } else if (cell == L'#') {
                        out += "\x1B[2;32m#\x1B[0m";
                    } else if (cell == L'E') {
                        out += "\x1B[1;33mE\x1B[0m";
                    } else if (cell == L'>' || cell == L'<' ||
                               cell == L'^' || cell == L'v') {
                        out += std::string("\x1B[33m") + (char)cell + "\x1B[0m";
                    } else if (cell == L'R') {
                        out += "\x1B[36mR\x1B[0m";
                    } else {
                        out += "\x1B[2m.\x1B[0m";
                    }
                }
            }
        }

        std::cout << out;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // ─── Exit erreicht → Abschluss-Screen, zurück zum Menü ───────────────────
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\x1B[?25h\x1B[0m\x1B[2J\x1B[H" << std::flush;

    {
        auto doc = vbox({
            text(""),
            text("You escaped the Matrix!") | bold | color(Color::Yellow) | center,
            text(""),
            text(R"("The rabbit hole goes deeper...")")
                | italic | color(Color::GreenLight) | center,
            text(""),
            text("[ Zurück zum Menü... ]") | dim | color(Color::Cyan) | center,
            text(""),
        }) | border | color(Color::Green);

        auto scr = Screen::Create(Dimension::Full(), Dimension::Fit(doc));
        Render(scr, doc);
        scr.Print();
        std::cout << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    }

    return 0;
}
