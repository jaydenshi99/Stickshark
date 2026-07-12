#pragma once

#include <climits>
#include <algorithm>
#include <chrono>

#include "../agent/agent.h"
#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"
#include "../constants.h"

struct Node {
    uint32_t firstChild = 0;   // index into arena; 0 = not expanded yet
    uint16_t numChildren = 0;
    uint16_t move = 0;         // 16-bit moveValue. the edge from parent
    uint32_t visits = 0;
    float    valueSum = 0.0f;  // sum of results, side-to-move perspective
};

class MCTS : public Agent {
    private:
    // exploration constant
    float C = 1.5f; 

    vector<Node> arena;
    vector<uint32_t> path;     // arena indices of the current descent, root first

    // xorshift64 RNG for rollouts (fixed seed = reproducible searches)
    uint64_t rngState = 0x9E3779B97F4A7C15ull;
    inline uint64_t nextRandom() {
        rngState ^= rngState << 13;
        rngState ^= rngState >> 7;
        rngState ^= rngState << 17;
        return rngState;
    }

    float monteCarloTreeSearch(int numTrials);
    uint32_t selectChild(uint32_t parentIdx);
    float rollout();

    public:
    // Constructor Destructor
    MCTS(Board b);
    ~MCTS();

    // Agent interface
    void findBestMove(int softLimit, int hardLimit, int maxDepth = MAX_PLY) override;
    void setPosition(Board b) override;
    void reset(Board b) override;


};