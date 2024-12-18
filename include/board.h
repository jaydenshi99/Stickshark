#pragma once

#include <cstdint>

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

class Board {
    public:
    // Board Data
    uint64_t pieces[12];
    bool turn; // true - white | false - black

    // Get methods
    uint64_t getBoardPositions() const;
    uint64_t getWhitePositions() const;
    uint64_t getBlackPositions() const;

    // Set methods
    void swapTurn();
};