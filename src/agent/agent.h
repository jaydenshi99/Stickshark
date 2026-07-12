#pragma once

#include <functional>
#include <vector>

#include "../chess/board/board.h"
#include "../chess/board/move.h"
#include "../constants.h"

// Callback used to report search progress to a UCI frontend.
using UciInfoCallback = std::function<void(int depth, int timeMs, int nodes, int nps, int scoreCp, const std::vector<Move>& pv)>;

// Callback for free-form search info, relayed as a UCI "info string" line.
using InfoStringCallback = std::function<void(const std::string&)>;

// Abstract base class for move-selecting agents (alpha-beta Engine, MCTS, ...).
// An agent owns a board position and, when asked, searches it and exposes the
// result through bestMove.
class Agent {
    public:
    Board board;
    Move bestMove;

    Agent() = default;
    Agent(Board b) : board(b) {}
    virtual ~Agent() = default;

    // Search the current position within the given time budget (milliseconds)
    // and store the chosen move in bestMove.
    virtual void findBestMove(int softLimit, int hardLimit, int maxDepth = MAX_PLY) = 0;

    // Set a new position, keeping any accumulated search state (e.g. TT).
    virtual void setPosition(Board b) = 0;

    // Set a new position and clear all accumulated search state.
    virtual void reset(Board b) = 0;

    // Optional UCI hooks; agents without search stats / info reporting keep the no-ops.
    virtual void setUciInfoCallback(UciInfoCallback callback) { (void)callback; }
    virtual void setInfoStringCallback(InfoStringCallback callback) { (void)callback; }
    virtual void resetSearchStats() {}
};
