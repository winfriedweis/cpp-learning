#pragma once
#include <string>
#include <vector>

class RayCaster {
public:
    int run();
    static const char* getName() { return "RayCaster - Game"; }

private:
    std::vector<char>  screen_;       // shade index per cell (0–4)
    std::vector<char>  wallTypeBuf_;  // wall type per cell
    std::vector<float> uBuf_;         // texture U coordinate per screen column (0..1)
    std::vector<int>   ceilBuf_;      // ceiling row per screen column
};
