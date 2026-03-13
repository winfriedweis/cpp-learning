#pragma once
#include <string>

class RayCaster {
public:
    int run();
    static const char* getName() { return "RayCaster - Game"; }

private:
    std::string screen_;       // shade index per cell (0–4)
    std::string wallTypeBuf_;  // wall type per cell ('f'=floor, '#'=wall, '>'<'^''v'=arrow, 'R'=rabbit, 'E'=exit)
};
