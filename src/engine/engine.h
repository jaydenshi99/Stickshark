#pragma once

#include <climits>
#include <algorithm>
#include <functional>
#include <chrono>

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
    int tableProbes;
    int tableProbesQuiescence;
    int tableUsefulHits;
    int tableUsefulHitsQuiescence;
    
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    int timeLimit;

    bool searchFinished;

    TranspositionTable* TT;
    
    // UCI info reporting
    std::function<void(int depth, int timeMs, int nodes, int nps, int scoreCp, const std::vector<Move>& pv)> uciInfoCallback;
    
    // Principal variation tracking
    std::vector<Move> principalVariation;

    public:
    Board board;
    Move bestMove;

    int16_t boardEval;

    // Constructor
    Engine(Board b);
    ~Engine();

    // Set methods
    void resetEngine(Board b);
    void setPosition(Board b);  // Set position without clearing TT

    void findBestMove(int y);   // Calls negaMax to find the best move and debugs.
    int16_t negaMax(int depth, int16_t alpha, int16_t beta, int16_t turn, bool isRoot = false);    // Sets bestMove to the best move and sets moveEval to the eva
    int16_t quiescenceSearch(int16_t alpha, int16_t beta, int16_t turn);
    
    // UCI interface
    void setUciInfoCallback(std::function<void(int depth, int timeMs, int nodes, int nps, int scoreCp, const std::vector<Move>& pv)> callback);
    const std::vector<Move>& getPrincipalVariation() const { return principalVariation; }
    void resetSearchStats();

    // Helper
    inline bool isTimeUp() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        return elapsedTime >= timeLimit;
    }

private:
    void setFinalResult(int16_t score, Move& move);
};