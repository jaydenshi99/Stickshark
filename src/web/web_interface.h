#pragma once

#include <string>
#include <sstream>
#include "chess/board/board.h"
#include "engine/engine.h"
#include "utility.h"
#include "constants.h"

class WebInterface {
private:
    Engine* engine;
    bool quiet = false; // suppress engine stdout when true
    
public:
    WebInterface();
    ~WebInterface();
    
    // Control verbosity
    inline void setQuiet(bool q) { quiet = q; }
    
    // Main entry point for web commands
    std::string processCommand(const std::string& input);
    
    // Individual command handlers
    std::string handleNewGame();
    std::string handleMove(const std::string& moveStr);
    std::string handleEngineMove(int timeMs = 1000);
    std::string handleGetBoard();
    
    // Utility functions
    std::string boardToJson() const;
    std::string errorResponse(const std::string& message) const;
    void writeStateToFile(const std::string& filename, const std::stringstream& state) const;
};
