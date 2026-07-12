#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "agent/agent.h"
#include "engine/engine.h"
#include "engine/openingBook.h"
#include "mcts/mcts.h"
#include "chess/board/board.h"

// Minimal UCI driver separate from HTTP/Web interface.
// Supports: uci, isready, ucinewgame, position (startpos|fen ... [moves ...]), go, stop, quit,
// setoption name SearchAlgorithm value (AlphaBeta|MCTS)
class UCI {
public:
    UCI(bool useMcts = false);
    ~UCI();

    // Run a blocking UCI loop on stdin/stdout
    void loop();

private:
    Agent* agent;
    bool usingMcts;
    OpeningBook book;
    std::streambuf* orig_cout;  // Store original cout streambuf for UCI responses

    // Custom streambuf that discards all output
    class NullBuffer : public std::streambuf {
    public:
        int overflow(int c) { return c; }
    };
    NullBuffer nullBuffer;

    // Command handlers
    void handlePosition(const std::string& line);
    void handleGo(const std::string& line);
    void handleSetOption(const std::string& line);

    // Swap the active agent (alpha-beta Engine or MCTS), preserving the current position
    void setAgentType(bool mcts);
    void attachInfoCallback();

    // Helpers
    static std::string moveToUci(const Move& m);
    static bool parseUciMoveToken(const std::string& token, int& src, int& dst, int& promoFlag);
    static bool findLegalMoveBySquares(const Board& board, int src, int dst, int promoFlag, Move& out);
};
