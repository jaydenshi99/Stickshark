#pragma once

#include <vector>
#include <cstdint>

#include "../board/board.h"
#include "../bit.h"
#include "../../utility.h"
#include "bitTables.h"

#define MAX_MOVES 300 // Arbitrary

class MoveGen {
    private:
    // Storing data that is used accross all functions
    uint64_t friendly;
    uint64_t enemyOrEmpty;

    // These functions require the working of generatePseudoMoves
    void generatePawnMoves(const Board& b);
    void generateKnightMoves(const Board& b);
    void generateSlidingMoves(const Board& b);
    void generateKingMoves(const Board& b);

    public:
    std::vector<Move> moves;

    // Constructor
    MoveGen();

    void generatePseudoMoves(const Board& b);



    void clearMoves();
};