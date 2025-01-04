#pragma once

#include <climits>

#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"

class Engine {
    private:
    MoveGen mg;

    Move bestMove;
    int boardEval;

    int searchDepth;

    public:
    Board board;

    // Constructor
    Engine(Board b);

    // Set methods
    void setBoard(Board b);

    void findBestMove(int depth);   // Calls negaMax to find the best move and debugs.
    int negaMax(int depth);    // Sets bestMove to the best move and sets moveEval to the eva
};