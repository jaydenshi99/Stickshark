#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <stack>

#include "move.h"
#include "gamestate.h"
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

#define NUM_PIECES 12

class Board {
    public:
    // Board Data
    uint64_t pieceBitboards[NUM_PIECES];
    uint64_t attackBitboards[NUM_PIECES];
    int squares[64];
    bool turn; // true - white | false - black

    // Gamestate History
    std::stack<Gamestate> history;

    // Array of set attack methods
    void (Board::*setAttackMethods[6])(bool white);

    // Constructor
    Board();

    // Get methods
    uint64_t getBlockers() const;
    uint64_t getWhitePositions() const;
    uint64_t getBlackPositions() const;

    uint64_t getWhiteAttacks() const;
    uint64_t getBlackAttacks() const;

    // Set methods
    void setStartingPosition();
    void swapTurn();

    void makeMove(Move move);
    void unmakeMove(Move move);

    // Set attack bitboards
    void updatePieceAttacks(int piece);
    
    void setPawnAttacks(bool white);
    void setKnightAttacks(bool white);
    void setKingAttacks(bool white);
    void setSliderAttacks();

    // Display methods
    void displayBoard() const;
};

bool isNonSliding(int piece);