#pragma once

#include <cstdint>

#define NUM_PIECES 12

class Gamestate {
    public:
    // Constructor
    Gamestate(int cptPiece);

    // Data
    int capturedPiece;

    uint64_t attackBitboards[NUM_PIECES];
};