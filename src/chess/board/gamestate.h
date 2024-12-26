#pragma once

#include <cstdint>

#include "../moveGeneration/bitTables.h"

#define EMPTY   -1
#define WPAWN   0
#define WBISHOP 1
#define WKNIGHT 2
#define WROOK   3
#define WQUEEN  4
#define WKING   5
#define BPAWN   6
#define BBISHOP 7
#define BKNIGHT 8
#define BROOK   9
#define BQUEEN  10
#define BKING   11

class Gamestate {
    public:
    // Constructor
    Gamestate(uint64_t pieceBitboards[12], int cptPiece);

    // Data
    int capturedPiece;
    uint64_t whiteAttacks;  // Set on initialisation
    uint64_t blackAttacks;  // Set on initialisation
};