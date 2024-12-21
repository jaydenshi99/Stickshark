#pragma once

#include <vector>
#include <cstdint>

#include "../board/board.h"
#include "../bit.h"

class MoveGen {
    public:
    std::vector<Move> moves;

    void generatePawnMoves(Board b);
};