#pragma once

#include <string>
#include <sstream>
#include "chess/board/board.h"
#include "engine/engine.h"
#include "utility.h"
#include "constants.h"

class WebInterface {
private:
    Board board;
    Engine* engine;
    
public:
    WebInterface();
    ~WebInterface();
    
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
};
