#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <stack>

#include "move.h"
#include "gamestate.h"
#include "../moveGeneration/bitTables.h"
#include "../../constants.h"

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

    uint64_t blockers;

    int squares[64];

    bool turn; // true - white | false - black

    // Gamestate History
    std::stack<Gamestate> history;

    // Array of set attack methods
    void (Board::*setAttackMethods[6])(Gamestate& gamestate, bool white);

    // Constructor
    Board();

    // Get methods
    uint64_t getWhitePositions() const;
    uint64_t getBlackPositions() const;

    uint64_t getWhiteAttacks() const;
    uint64_t getBlackAttacks() const;

    bool kingInCheck() const;

    // Set methods
    void setFEN(std::string FEN);
    void setStartingPosition();
    void swapTurn();

    void makeMove(const Move& move);
    void unmakeMove(const Move& move);

    // Set attack bitboards
    void setBlockers();

    void updatePieceAttacks(Gamestate& gamestate, int piece);
    
    void setPawnAttacks(Gamestate& gamestate, bool white);
    void setKnightAttacks(Gamestate& gamestate, bool white);
    void setKingAttacks(Gamestate& gamestate, bool white);
    void setSliderAttacks(Gamestate& gamestate);

    // Display methods
    void displayBoard() const;
};

int charToPieceIndex (char c);