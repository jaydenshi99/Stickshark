#pragma once

#include <climits>
#include <algorithm>

#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"
#include "evaluation.h"
#include "../constants.h"

class Engine {
    private:
    int searchDepth;

    int leafNodesEvaluated;
    int tableAccesses;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    int timeLimit;

    bool searchFinished;

    std::unordered_map<uint64_t, uint16_t> bestMoveTable;

    public:
    Board board;
    Move bestMove;

    int boardEval;

    // Constructor
    Engine(Board b);

    // Set methods
    void resetEngine(Board b);

    void findBestMove(int y);   // Calls negaMax to find the best move and debugs.
    int negaMax(int depth, int alpha, int beta, int turn);    // Sets bestMove to the best move and sets moveEval to the eva

    // Helper
    inline bool isTimeUp() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        return elapsedTime >= timeLimit;
    }

    // Hashmaps
    void storeBestMove(uint64_t zobristKey, uint16_t moveValue);
    bool retrieveBestMove(uint64_t zobristKey, uint16_t& moveValue) const; 
};