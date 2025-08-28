#include "web_interface.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

using namespace std;

WebInterface::WebInterface() {
    board = Board();
    board.setFEN(STARTING_FEN);
    engine = new Engine(board);
}

WebInterface::~WebInterface() {
    delete engine;
}

string WebInterface::processCommand(const string& input) {
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
    board.setFEN(STARTING_FEN);
    engine->setBoard(board);
    
    // Write state to file for HTML visualizer
    writeStateToFile();
    
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"newgame\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (board.turn ? "white" : "black") << "\"";
    response << "}";
    
    return response.str();
}

string WebInterface::handleMove(const string& moveStr) {
    // Parse move in format "e2-e4"
    if (moveStr.length() != 5 || moveStr[2] != '-') {
        return errorResponse("Invalid move format. Expected: e2-e4");
    }
    
    // Convert to Move object using existing utility
    Move move = notationToMove(moveStr);
    
    // Apply the move
    board.makeMove(move);
    engine->setBoard(board);
    
    // Write state to file for HTML visualizer
    writeStateToFile();
    
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"move\",";
    response << "\"move\": \"" << moveStr << "\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (board.turn ? "white" : "black") << "\"";
    response << "}";
    
    return response.str();
}

string WebInterface::handleEngineMove(int timeMs) {
    // Suppress engine debug output by redirecting cout temporarily
    streambuf* orig = cout.rdbuf();
    stringstream devnull;
    cout.rdbuf(devnull.rdbuf());
    
    // Get engine move
    engine->findBestMove(timeMs);
    Move bestMove = engine->bestMove;
    board.makeMove(bestMove);
    
    // Restore cout
    cout.rdbuf(orig);
    
    // Write state to file for HTML visualizer
    writeStateToFile();
    
    // Convert move to notation
    char fromFile = 'a' + (bestMove.getSource() % 8);
    char fromRank = '1' + (bestMove.getSource() / 8);
    char toFile = 'a' + (bestMove.getTarget() % 8);
    char toRank = '1' + (bestMove.getTarget() / 8);
    
    string moveNotation = string(1, fromFile) + string(1, fromRank) + "-" + 
                         string(1, toFile) + string(1, toRank);
    
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"enginemove\",";
    response << "\"move\": \"" << moveNotation << "\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (board.turn ? "white" : "black") << "\"";
    response << "}";
    
    return response.str();
}

string WebInterface::handleGetBoard() {
    // Write state to file for HTML visualizer
    writeStateToFile();
    
    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"getboard\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (board.turn ? "white" : "black") << "\"";
    response << "}";
    
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
            int piece = board.squares[square];
            
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
    stringstream response;
    response << "{";
    response << "\"status\": \"error\",";
    response << "\"message\": \"" << message << "\"";
    response << "}";
    
    return response.str();
}

void WebInterface::writeStateToFile(const string& filename) const {
    ofstream file(filename);
    if (file.is_open()) {
        stringstream state;
        state << "{";
        state << "\"status\": \"success\",";
        state << "\"action\": \"fileupdate\",";
        state << "\"board\": " << boardToJson() << ",";
        state << "\"turn\": \"" << (board.turn ? "white" : "black") << "\",";
        state << "\"timestamp\": " << time(nullptr);
        state << "}";
        
        file << state.str() << endl;
        file.close();
    }
}
