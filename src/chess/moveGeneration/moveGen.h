#pragma once

#include <vector>
#include <cstdint>

#include "../board/board.h"
#include "../bit.h"
#include "../../utility.h"
#include "bitTables.h"

#define MAX_MOVES 200 // Arbitrary

class MoveGen {
    public:
    std::vector<Move> moves;

    // Constructor
    MoveGen();

    void generatePseudoMoves(const Board& b);

    void generatePawnMoves(const Board& b);
    void generateKnightMoves(const Board& b);
    void generateSlidingMoves(const Board& b);
    void generateKingMoves(const Board& b);

    void clearMoves();
};