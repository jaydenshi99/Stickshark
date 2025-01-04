#pragma once

#include "../board/board.h"

class Engine {
    private:
    Move bestMove;
    int moveEval;

    public:
    Board board;

    // Constructor
    Engine(Board b);

    // Set methods
    void calculateBestMove(int depth); // 
    int negamax(int depth);    // Sets bestMove to the best move and sets moveEval to the eva
};