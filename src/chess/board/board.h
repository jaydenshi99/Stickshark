#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include "move.h"
#include "gamestate.h"
#include "../moveGeneration/bitTables.h"
#include "../../constants.h"
#include "../../engine/evalConstants.h"

using namespace std;

class Board {
    public:
    // Board Data
    uint64_t pieceBitboards[NUM_PIECES];

    uint64_t blockers;

    int squares[64];

    bool turn; // true - white | false - black

    // Gamestate History
    stack<Gamestate> history;

    // Repetition Tracking
    unordered_map<uint64_t, int> repetitionCount;

    // Zobrist Hash
    uint64_t zobristHash;

    // Evaluation 
    int pieceSquareEvalMG; // Positive is better for white
    int pieceSquareEvalEG; // Positive is better for white

    // Array of set attack methods
    void (Board::*setAttackMethods[6])(Gamestate& gamestate, bool white);

    // Constructor
    Board();

    // Get methods (inlined for hot-path performance)
    inline __attribute__((always_inline)) uint64_t getWhitePositions() const {
        return pieceBitboards[WPAWN] | pieceBitboards[WBISHOP] | pieceBitboards[WKNIGHT] |
               pieceBitboards[WROOK] | pieceBitboards[WQUEEN] | pieceBitboards[WKING];
    }
    inline __attribute__((always_inline)) uint64_t getBlackPositions() const {
        return pieceBitboards[BPAWN] | pieceBitboards[BBISHOP] | pieceBitboards[BKNIGHT] |
               pieceBitboards[BROOK] | pieceBitboards[BQUEEN] | pieceBitboards[BKING];
    }
    inline __attribute__((always_inline)) uint64_t getWhiteAttacks() const {
        const Gamestate& gamestate = history.top();
        return gamestate.attackBitboards[WPAWN] | gamestate.attackBitboards[WBISHOP] |
               gamestate.attackBitboards[WKNIGHT] | gamestate.attackBitboards[WROOK] |
               gamestate.attackBitboards[WQUEEN] | gamestate.attackBitboards[WKING];
    }
    inline __attribute__((always_inline)) uint64_t getBlackAttacks() const {
        const Gamestate& gamestate = history.top();
        return gamestate.attackBitboards[BPAWN] | gamestate.attackBitboards[BBISHOP] |
               gamestate.attackBitboards[BKNIGHT] | gamestate.attackBitboards[BROOK] |
               gamestate.attackBitboards[BQUEEN] | gamestate.attackBitboards[BKING];
    }
    inline __attribute__((always_inline)) bool kingInCheck() const {
        return pieceBitboards[turn ? BKING : WKING] & (turn ? getWhiteAttacks() : getBlackAttacks());
    }

    inline __attribute__((always_inline)) uint64_t getPieceAttacks(int piece) const {
        const Gamestate& gamestate = history.top();
        return gamestate.attackBitboards[piece];
    }

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

    void setZobristHash();

    void setPieceSquareEvaluation();

    // Display methods
    void displayBoard() const;
};

int charToPieceIndex (char c);