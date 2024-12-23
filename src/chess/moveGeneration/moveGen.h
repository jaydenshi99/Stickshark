#pragma once

#include <vector>
#include <cstdint>

#include "../board/board.h"
#include "../bit.h"
#include "../../utility.h"
#include "bitTables.h"

class MoveGen {
    public:
    std::vector<Move> moves;

    // Constructor
    MoveGen();

    void generatePseudoMoves(Board b);

    void generatePawnMoves(Board b);
    void generateKnightMoves(Board b);

    void clearMoves();
};