#pragma once

#include <climits>
#include <algorithm>
#include <functional>
#include <chrono>

#include "../agent/agent.h"
#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"
#include "evaluation.h"
#include "../constants.h"

class TranspositionTable;

class Engine : public Agent {
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

    uint16_t killerMoves[2][MAX_PLY];
    int killerHistory[2][NUM_SQUARES][NUM_SQUARES];
    int lmrTable[MAX_PLY][MAX_PLY];
    
    // UCI info reporting
    UciInfoCallback uciInfoCallback;
    
    // Principal variation tracking
    std::vector<Move> principalVariation;

    public:
    int16_t boardEval;

    // Constructor
    Engine(Board b);
    ~Engine();

    // Set methods
    void reset(Board b) override;
    void setPosition(Board b) override;  // Set position without clearing TT

    void findBestMove(int softLimit, int hardLimit, int maxDepth = MAX_PLY) override;
    int16_t negaMax(int depth, int ply, int16_t alpha, int16_t beta, int16_t turn, bool isRoot = false);    // Sets bestMove to the best move and sets moveEval to the eva
    int16_t quiescenceSearch(int16_t alpha, int16_t beta, int16_t turn, int ply);
    
    // UCI interface
    void setUciInfoCallback(UciInfoCallback callback) override;
    const std::vector<Move>& getPrincipalVariation() const { return principalVariation; }
    void resetSearchStats() override;

    // Helper
    inline bool isTimeUp() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        return elapsedTime >= timeLimit;
    }

private:
    void setFinalResult(int16_t score, Move& move);
    void buildPrincipalVariation();  // reconstruct full PV by walking the TT
};