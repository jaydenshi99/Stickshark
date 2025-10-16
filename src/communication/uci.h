#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "engine/engine.h"
#include "chess/board/board.h"

// Minimal UCI driver separate from HTTP/Web interface.
// Supports: uci, isready, ucinewgame, position (startpos|fen ... [moves ...]), go, stop, quit
class UCI {
public:
    UCI();
    ~UCI();

    // Run a blocking UCI loop on stdin/stdout
    void loop();

private:
    Engine* engine;
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

    // Helpers
    static std::string moveToUci(const Move& m);
    static bool parseUciMoveToken(const std::string& token, int& src, int& dst, int& promoFlag);
    static bool findLegalMoveBySquares(const Board& board, int src, int dst, int promoFlag, Move& out);
};


