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
    void debug(const std::string& msg) const;
public:
    WebInterface();
    ~WebInterface();
    inline void setQuiet(bool q) { quiet = q; }
    std::string processCommand(const std::string& input);
    std::string handleNewGame();
    std::string handleMove(const std::string& moveStr);
    std::string handleValidatedMove(const std::string& body);
    std::string handleEngineMove(int timeMs = 1000);
    std::string handleGetBoard();
    std::string handleGetLegal();
    std::string boardToJson() const;
    std::string errorResponse(const std::string& message) const;
    void writeStateToFile(const std::string& filename, const std::stringstream& state) const;
};


