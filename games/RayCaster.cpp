#include "RayCaster.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <string>
#include <algorithm>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <conio.h>
#else
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <unistd.h>
#endif

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// ── Platform helpers ──────────────────────────────────────────────────────────

static void getTermSize(int& w, int& h) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        w = csbi.srWindow.Right  - csbi.srWindow.Left;   // -1 col safety margin
        h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else { w = 119; h = 40; }
#else
    struct winsize ws{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    w = (ws.ws_col > 0 ? ws.ws_col - 1 : 119);
    h = (ws.ws_row > 0 ? ws.ws_row     : 40);
#endif
}

static void enableAnsi() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  m = 0;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

// ── Textured pixel renderer ───────────────────────────────────────────────────

// Rabbit silhouette: returns true if UV position is inside the rabbit shape
static bool rabbitShape(float u, float v) {
    // Ears: two thin rectangles at the top 22% of the cell
    if (v < 0.22f)
        return (u > 0.22f && u < 0.40f) || (u > 0.60f && u < 0.78f);

    // Head: oval from v=0.22 to v=0.55
    if (v < 0.55f) {
        float dx = (u - 0.50f) / 0.42f;
        float dy = (v - 0.385f) / 0.175f;
        return dx*dx + dy*dy <= 1.0f;
    }

    // Body: wider oval from v=0.55 to v=1.0
    float dx = (u - 0.50f) / 0.46f;
    float dy = (v - 0.78f)  / 0.25f;
    return dx*dx + dy*dy <= 1.0f;
}

// Returns a colored UTF-8 character for wall/floor pixel at texture coords (u, v).
// shade 1–4 controls block density; u,v in [0,1] are UV within the wall cell.
static std::string texturedPixel(char shade, char wtype, float u, float v) {
    if (shade == 0) return " ";

    static const char* blk[] = {
        " ",
        "\xE2\x96\x91",  // ░
        "\xE2\x96\x92",  // ▒
        "\xE2\x96\x93",  // ▓
        "\xE2\x96\x88",  // █
    };

    // Clamp shade to valid range
    int s = std::max(1, std::min(4, (int)shade));

    // Helper: ANSI color + block + reset
    auto col = [&](const char* ansi, int sh) -> std::string {
        int cs = std::max(1, std::min(4, sh));
        return std::string(ansi) + blk[cs] + "\x1B[0m";
    };

    // ── Floor ────────────────────────────────────────────────────────────────
    if (wtype == 'f')
        return col("\x1B[32m", s);

    // ── EXIT door: golden arch with frame ────────────────────────────────────
    if (wtype == 'E') {
        bool leftPost  = (u < 0.09f);
        bool rightPost = (u > 0.91f);
        bool topBar    = (v < 0.07f);
        bool botBar    = (v > 0.93f);
        if (leftPost || rightPost || topBar || botBar) {
            char ch = (topBar || botBar) ? '=' : '|';
            return std::string("\x1B[1;33m") + ch + "\x1B[0m";
        }
        // Interior golden glow (brigher near center)
        float cx = fabsf(u - 0.5f) * 2.0f;   // 0 = center, 1 = edge
        int gs = std::max(1, s - (int)(cx * 2));
        return col("\x1B[33m", gs);
    }

    // ── Rabbit sign: cyan silhouette ─────────────────────────────────────────
    if (wtype == 'R') {
        if (rabbitShape(u, v))
            return col("\x1B[1;36m", s);
        // Sign background: dark wooden panel
        return col("\x1B[2;33m", std::max(1, s - 1));
    }

    // ── Direction arrow signs: wooden board with big arrow glyph ────────────
    if (wtype == '>' || wtype == '<' || wtype == '^' || wtype == 'v') {
        // Arrow glyph in center band
        if (u > 0.30f && u < 0.70f && v > 0.25f && v < 0.75f)
            return std::string("\x1B[1;33m") + wtype + "\x1B[0m";
        // Wooden sign border/background
        return col("\x1B[33m", std::max(1, s - 1));
    }

    // ── Regular wall (#): staggered brick pattern ────────────────────────────
    {
        // 3 bricks wide, 4 rows tall; alternate rows are offset by half a brick
        float row = floorf(v * 4.0f);
        float bu  = u * 3.0f;
        if ((int)row % 2 == 1) bu = fmodf(bu + 0.5f, 3.0f);
        float uu = fmodf(bu, 1.0f);
        float vv = fmodf(v * 4.0f, 1.0f);

        bool mortar = (vv < 0.13f || uu < 0.06f);
        if (mortar)
            return col("\x1B[2;37m", std::max(1, s - 1));  // dim mortar
        return col("\x1B[37m", s);                         // bright brick
    }
}

// ── Main game loop ────────────────────────────────────────────────────────────

int RayCaster::run()
{
    enableAnsi();

    // ─── Terminal-Größe ───────────────────────────────────────────────────────
    int nScreenWidth, nScreenHeight;
    getTermSize(nScreenWidth, nScreenHeight);

    screen_.assign(nScreenWidth * nScreenHeight, 0);
    wallTypeBuf_.assign(nScreenWidth * nScreenHeight, 0);
    uBuf_.assign(nScreenWidth, 0.0f);
    ceilBuf_.assign(nScreenWidth, 0);

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
    map += L"# #   v # #    #";  // 5  ← Pfeil-Schild: nach unten
    map += L"# ### # # # ## #";  // 6
    map += L"#   # #   #  # #";  // 7
    map += L"### # ##### ## #";  // 8
    map += L"#   #   >   #  #";  // 9  ← Pfeil-Schild: nach rechts
    map += L"# ##### # ### ##";  // 10
    map += L"# #   R #   #  #";  // 11 ← Hase-Schild (cyan)
    map += L"# # ### ##### ##";  // 12
    map += L"# #     #      #";  // 13
    map += L"#   #####      E";  // 14 ← EXIT-Tür (gold)
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

#ifdef _WIN32
        while (_kbhit()) _getch();  // flush input buffer
        while (!_kbhit()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
        _getch();
#else
        tcflush(STDIN_FILENO, TCIFLUSH);
        std::cin.get();
#endif
    }

    // ─── Raw Mode (POSIX only) ────────────────────────────────────────────────
#ifndef _WIN32
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN]  = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif

    const int kHoldFrames = 4;
    int lastW = -100, lastS = -100, lastA = -100, lastD = -100;
    int frame = 0;

    // Nur '#' ist eine feste Wand; alle anderen Zeichen (inkl. Schilder) sind begehbar
    auto isWalkable = [&](int mx, int my) -> bool {
        if (mx < 0 || mx >= nMapWidth || my < 0 || my >= nMapHeight) return false;
        return map[my * nMapWidth + mx] != L'#';
    };

    // ─── Game Loop ────────────────────────────────────────────────────────────
    while (true) {
        frame++;

        // ── Input ─────────────────────────────────────────────────────────────
        bool kQuit = false;
#ifdef _WIN32
        while (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) { if (_kbhit()) _getch(); continue; }
            char c = (char)ch;
            if (c == 'w' || c == 'W') lastW = frame;
            if (c == 's' || c == 'S') lastS = frame;
            if (c == 'a' || c == 'A') lastA = frame;
            if (c == 'd' || c == 'D') lastD = frame;
            if (c == 'q' || c == 27)  kQuit = true;
        }
#else
        char c;
        while (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'w' || c == 'W') lastW = frame;
            if (c == 's' || c == 'S') lastS = frame;
            if (c == 'a' || c == 'A') lastA = frame;
            if (c == 'd' || c == 'D') lastD = frame;
            if (c == 'q' || c == 27)  kQuit = true;
        }
#endif

        bool kW = (frame - lastW) <= kHoldFrames;
        bool kS = (frame - lastS) <= kHoldFrames;
        bool kA = (frame - lastA) <= kHoldFrames;
        bool kD = (frame - lastD) <= kHoldFrames;

        if (kQuit) {
#ifndef _WIN32
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
            std::cout << "\x1B[?25h\x1B[0m\n";
            return 0;
        }

        if (kA) fPlayerA -= 0.03f;
        if (kD) fPlayerA += 0.03f;

        // ── Bewegung mit Wall-Sliding ──────────────────────────────────────────
        bool exitReached = false;
        const float fMoveSpeed = 0.08f;

        auto tryMove = [&](float dx, float dy) {
            float nx = fPlayerX + dx;
            float ny = fPlayerY + dy;
            int mx = (int)nx, my = (int)ny;
            if (!isWalkable(mx, my)) {
                // Wall-Sliding: versuche Einzelachsen
                if (isWalkable((int)(fPlayerX + dx), (int)fPlayerY)) {
                    nx = fPlayerX + dx; ny = fPlayerY; mx = (int)nx; my = (int)ny;
                } else if (isWalkable((int)fPlayerX, (int)(fPlayerY + dy))) {
                    nx = fPlayerX; ny = fPlayerY + dy; mx = (int)nx; my = (int)ny;
                } else { return; }
            }
            wchar_t cell = map[my * nMapWidth + mx];
            if (cell == L'E') { exitReached = true; return; }
            fPlayerX = nx;
            fPlayerY = ny;
        };

        if (kW) tryMove( sinf(fPlayerA) * fMoveSpeed,  cosf(fPlayerA) * fMoveSpeed);
        if (kS) tryMove(-sinf(fPlayerA) * fMoveSpeed, -cosf(fPlayerA) * fMoveSpeed);

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
                    if (cell != L' ') {
                        bHitWall = true;
                        hitCell  = cell;
                    }
                }
            }

            // Fish-Eye-Korrektur: perpendicular distance für gerade Wände
            float fPerpDist = fDistanceToWall * cosf(fRayAngle - fPlayerA);
            if (fPerpDist < 0.0001f) fPerpDist = 0.0001f;

            // Shade basiert auf roher Ray-Distanz (Nebeleffekt)
            char nShade = 0;
            if      (fDistanceToWall <= fDepth / 4.0f) nShade = 4;
            else if (fDistanceToWall <  fDepth / 3.0f) nShade = 3;
            else if (fDistanceToWall <  fDepth / 2.0f) nShade = 2;
            else if (fDistanceToWall <  fDepth)         nShade = 1;

            const float fAspect = 0.5f;
            int nCeiling = (int)((float)nScreenHeight / 2.0f -
                                 (float)nScreenHeight * fAspect / fPerpDist);
            int nFloor   = nScreenHeight - nCeiling;

            // Texture U-Koordinate: horizontale Position im getroffenen Wandblock
            float hitX = fPlayerX + fEyeX * fDistanceToWall;
            float hitY = fPlayerY + fEyeY * fDistanceToWall;
            float cellFrac;
            if (fabsf(fEyeX) > fabsf(fEyeY))
                cellFrac = hitY - floorf(hitY);  // N/S-Wand: Y-Fraktion
            else
                cellFrac = hitX - floorf(hitX);  // E/W-Wand: X-Fraktion

            uBuf_[x]    = cellFrac;
            ceilBuf_[x] = nCeiling;

            char wtype = (hitCell < 128) ? (char)hitCell : '#';

            for (int y = 0; y < nScreenHeight; y++) {
                int idx = y * nScreenWidth + x;
                if (y < nCeiling) {
                    screen_[idx] = 0; wallTypeBuf_[idx] = 0;
                } else if (y >= nCeiling && y <= nFloor) {
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

        // ─── Frame-Output ──────────────────────────────────────────────────────
        std::string out = "\x1B[?25l\x1B[H";
        for (int y = 0; y < nScreenHeight; y++) {
            for (int x = 0; x < nScreenWidth; x++) {
                int   idx   = y * nScreenWidth + x;
                char  sh    = screen_[idx];
                char  wt    = wallTypeBuf_[idx];
                float u     = uBuf_[x];
                int   ceil_ = ceilBuf_[x];
                int   floor_= nScreenHeight - ceil_;
                // V-Koordinate: 0 = Oberkante Wand, 1 = Unterkante Wand
                float v = (floor_ > ceil_ && y >= ceil_ && y <= floor_)
                          ? (float)(y - ceil_) / (float)(floor_ - ceil_)
                          : 0.5f;
                out += texturedPixel(sh, wt, u, v);
            }
            out += '\n';
        }

        // ─── Minimap (ANSI absolut, oben rechts) ──────────────────────────────
        if (nScreenWidth > nMapWidth + 4) {
            int mmCol = nScreenWidth - nMapWidth + 1;

            float a = fmodf(fPlayerA, 6.28318f);
            if (a < 0) a += 6.28318f;
            char dirChar = (a < 0.785f || a >= 5.498f) ? 'v'
                         : (a < 2.356f)                ? '>'
                         : (a < 3.927f)                ? '^'
                         :                               '<';

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

    // ─── Exit erreicht → Abschluss-Screen ────────────────────────────────────
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
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
