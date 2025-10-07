#pragma once

#include <climits>
#include <algorithm>

#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"
#include "evaluation.h"
#include "../constants.h"

class TranspositionTable;

class Engine {
    private:
    int searchDepth;

    int normalNodesSearched;
    int quiescenceNodesSearched;
    int tableAccesses;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    int timeLimit;

    bool searchFinished;

    TranspositionTable* TT;

    public:
    Board board;
    Move bestMove;

    int16_t boardEval;

    // Constructor
    Engine(Board b);
    ~Engine();

    // Set methods
    void resetEngine(Board b);

    void findBestMove(int y);   // Calls negaMax to find the best move and debugs.
    int16_t negaMax(int depth, int16_t alpha, int16_t beta, int16_t turn);    // Sets bestMove to the best move and sets moveEval to the eva
    int16_t quiescenceSearch(int16_t alpha, int16_t beta, int16_t turn);

    // Helper
    inline bool isTimeUp() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        return elapsedTime >= timeLimit;
    }
};