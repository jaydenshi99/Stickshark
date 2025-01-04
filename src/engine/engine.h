#pragma once

#include <climits>
#include <algorithm>

#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"
#include "evaluation.h"
#include "../constants.h"

class Engine {
    private:
    int boardEval;

    int searchDepth;

    int leafNodesEvaluated;

    public:
    Board board;
    Move bestMove;

    // Constructor
    Engine(Board b);

    // Set methods
    void setBoard(Board b);

    void findBestMove(int depth);   // Calls negaMax to find the best move and debugs.
    int negaMax(int depth, int alpha, int beta, int turn);    // Sets bestMove to the best move and sets moveEval to the eva
};