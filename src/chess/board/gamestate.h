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
    uint8_t castlingRights; // 0 - wk | 1 - wq | 2 - bk | 3 - bq
};