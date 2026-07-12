#pragma once

#include <climits>
#include <algorithm>
#include <chrono>

#include "../agent/agent.h"
#include "../chess/board/board.h"
#include "../chess/moveGeneration/moveGen.h"
#include "../constants.h"

class MCTS : public Agent {
    public:
    // Constructor Destructor
    MCTS(Board b);
    ~MCTS();

    // Agent interface
    void findBestMove(int softLimit, int hardLimit, int maxDepth = MAX_PLY) override;
    void setPosition(Board b) override;
    void reset(Board b) override;

    float monteCarloTreeSearch(int numTrials);

};