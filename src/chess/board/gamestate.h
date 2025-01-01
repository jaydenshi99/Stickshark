#pragma once

#include <cstdint>

#include "../../constants.h"

class Gamestate {
    public:
    // Constructor
    Gamestate(int cptPiece);

    // Data
    int capturedPiece;

    uint64_t attackBitboards[NUM_PIECES];
};