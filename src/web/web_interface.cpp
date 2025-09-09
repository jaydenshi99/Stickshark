#include "web_interface.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

using namespace std;

WebInterface::WebInterface() {
    Board initialBoard;
    initialBoard.setFEN(STARTING_FEN);
    engine = new Engine(initialBoard);
}

WebInterface::~WebInterface() {
    delete engine;
}

void WebInterface::debug(const string& msg) const {
    if (!quiet) {
        cerr << "[WebInterface] " << msg << '\n';
    }
}

string WebInterface::processCommand(const string& input) {
    debug(string("processCommand: ") + input);
    // Simple command parsing - expects format: "COMMAND:data"
    size_t colonPos = input.find(':');
    
    if (colonPos == string::npos) {
        // Single word commands
        if (input == "newgame") {
            return handleNewGame();
        } else if (input == "getboard") {
            return handleGetBoard();
        } else if (input == "enginemove") {
            return handleEngineMove();
        } else {
            return errorResponse("Unknown command: " + input);
        }
    } else {
        // Commands with data
        string command = input.substr(0, colonPos);
        string data = input.substr(colonPos + 1);
        debug(string("command=") + command + ", data=" + data);
        
        if (command == "move") {
            return handleMove(data);
        } else if (command == "enginemove") {
            int timeMs = stoi(data);
            return handleEngineMove(timeMs);
        } else {
            return errorResponse("Unknown command: " + command);
        }
    }
}

string WebInterface::handleNewGame() {
    debug("handleNewGame");
    engine->board.setFEN(STARTING_FEN);
    
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"newgame\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (engine->board.turn ? "white" : "black") << "\",";
    response << "\"timestamp\": " << time(nullptr);
    response << "}";
    
    // Write state to file for HTML visualizer
    writeStateToFile("data/board_state.json", response);
    
    return response.str();
}

string WebInterface::handleMove(const string& moveStr) {
    debug(string("handleMove: ") + moveStr);
    // Parse move in format "e2-e4"
    if (moveStr.length() != 5 || moveStr[2] != '-') {
        return errorResponse("Invalid move format. Expected: e2-e4");
    }
    
    // Convert to Move object using existing utility
    Move move = notationToMove(moveStr, engine->board.turn);
    
    // Apply the move to engine's board
    engine->board.makeMove(move);
    
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"move\",";
    response << "\"move\": \"" << moveStr << "\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (engine->board.turn ? "white" : "black") << "\",";
    response << "\"timestamp\": " << time(nullptr);
    response << "}";
    
    // Write state to file for HTML visualizer
    writeStateToFile("data/board_state.json", response);
    
    return response.str();
}

string WebInterface::handleEngineMove(int timeMs) {
    debug(string("handleEngineMove: timeMs=") + to_string(timeMs));
    // Optionally suppress engine debug output when quiet
    streambuf* orig = nullptr;
    stringstream devnull;
    if (quiet) {
        orig = cout.rdbuf();
        cout.rdbuf(devnull.rdbuf());
    }
    
    // Get engine move (engine operates on its own board)
    engine->findBestMove(timeMs);
    Move bestMove = engine->bestMove;
    engine->board.makeMove(bestMove);

    if (quiet && orig) {
        cout.rdbuf(orig);
    }
    
    // Convert move to notation
    char fromFile = 'a' + (bestMove.getSource() % 8);
    char fromRank = '1' + (bestMove.getSource() / 8);
    char toFile = 'a' + (bestMove.getTarget() % 8);
    char toRank = '1' + (bestMove.getTarget() / 8);
    
    string moveNotation = string(1, fromFile) + string(1, fromRank) + "-" + 
                         string(1, toFile) + string(1, toRank);
    debug(string("engine best move: ") + moveNotation);

    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"enginemove\",";
    response << "\"move\": \"" << moveNotation << "\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (engine->board.turn ? "white" : "black") << "\",";

    int turn = engine->board.turn ? -1 : 1;

    response << "\"eval\": \"" << engine->boardEval * turn / 100.0 << "\",";
    response << "\"timestamp\": " << time(nullptr);
    response << "}";
    
    // Write state to file for HTML visualizer
    writeStateToFile("data/board_state.json", response);
    
    return response.str();
}

string WebInterface::handleGetBoard() {
    debug("handleGetBoard");
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"getboard\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (engine->board.turn ? "white" : "black") << "\",";
    response << "\"timestamp\": " << time(nullptr);
    response << "}";
    
    // Write state to file for HTML visualizer
    writeStateToFile("data/board_state.json", response);
    
    return response.str();
}

string WebInterface::boardToJson() const {
    stringstream json;
    json << "[";
    
    // Output board from rank 8 to rank 1 (top to bottom visually)
    for (int rank = 7; rank >= 0; rank--) {
        if (rank < 7) json << ",";
        json << "[";
        
        for (int file = 0; file < 8; file++) {
            if (file > 0) json << ",";
            
            int square = rank * 8 + file;
            int piece = engine->board.squares[square];
            
            if (piece == EMPTY) {
                json << "null";
            } else {
                // Piece types: 0=pawn, 1=bishop, 2=knight, 3=rook, 4=queen, 5=king
                int type = piece % 6;
                string color = piece < 6 ? "white" : "black";
                json << "{\"type\":" << type << ",\"color\":\"" << color << "\"}";
            }
        }
        json << "]";
    }
    json << "]";
    
    return json.str();
}

string WebInterface::errorResponse(const string& message) const {
    debug(string("error: ") + message);
    stringstream response;
    response << "{";
    response << "\"status\": \"error\",";
    response << "\"message\": \"" << message << "\",";
    response << "\"timestamp\": " << time(nullptr);
    response << "}";

    writeStateToFile("data/board_state.json", response);
    
    return response.str();
}

void WebInterface::writeStateToFile(const string& filename, const stringstream& state) const {
    ofstream file(filename);
    if (file.is_open()) {
        file << state.str() << endl;
        file.close();
    }
}
