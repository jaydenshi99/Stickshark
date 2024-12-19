#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <stack>

#include "move.h"
#include "gamestate.h"

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

class Board {
    public:
    // Constructor
    Board();

    // Board Data
    uint64_t pieceBitboards[12];
    int squares[64];
    bool turn; // true - white | false - black

    // Gamestate History
    stack<Gamestate> history;

    // Get methods
    uint64_t getBoardPositions() const;
    uint64_t getWhitePositions() const;
    uint64_t getBlackPositions() const;

    // Set methods
    void setStartingPosition();
    void swapTurn();

    void makeMove(Move move);
    void unmakeMove(Move move);

    // Display methods
    void displayBoard() const;
};