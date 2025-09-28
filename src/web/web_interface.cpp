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

    Board b;
    b.setFEN("4k3/8/8/8/8/8/5R2/4KR2 w - - 0 1");
    engine->resetEngine(b);
    
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

string WebInterface::handleValidatedMove(const string& body) {
    debug(string("handleValidatedMove: ") + body);
    // Parse JSON-ish body for id or from/to/flag; very lenient
    // Try id first
    string idStr;
    size_t posId = body.find("\"id\"");
    if (posId != string::npos) {
        size_t c = body.find(':', posId);
        size_t e = body.find_first_of(",}\n\r ", c + 1);
        idStr = body.substr(c + 1, e == string::npos ? string::npos : e - (c + 1));
        // trim
        idStr.erase(0, idStr.find_first_not_of(" \t\n\r"));
        if (!idStr.empty() && idStr.back() == ' ') idStr.pop_back();
    }

    MoveGen gen;
    vector<MoveGen::LegalMoveDTO> dtos;
    gen.getLegalMovesDTO(engine->board, dtos);

    const Move* chosen = nullptr;
    if (!idStr.empty()) {
        // numeric id
        uint16_t id = static_cast<uint16_t>(stoi(idStr));
        for (const Move& m : gen.legalMoves) {
            if (m.moveValue == id) { chosen = &m; break; }
        }
    } else {
        // Fallback: parse from/to/flag
        auto extractStr = [&](const char* key) -> string {
            string k = string("\"") + key + "\"";
            size_t p = body.find(k);
            if (p == string::npos) return "";
            size_t q1 = body.find('"', body.find(':', p) + 1);
            size_t q2 = q1 == string::npos ? string::npos : body.find('"', q1 + 1);
            if (q1 == string::npos || q2 == string::npos) return "";
            return body.substr(q1 + 1, q2 - q1 - 1);
        };
        auto extractInt = [&](const char* key) -> int {
            string k = string("\"") + key + "\"";
            size_t p = body.find(k);
            if (p == string::npos) return 0;
            size_t c = body.find(':', p);
            size_t e = body.find_first_of(",}\n\r ", c + 1);
            string v = body.substr(c + 1, e == string::npos ? string::npos : e - (c + 1));
            return stoi(v);
        };
        string from = extractStr("from");
        string to = extractStr("to");
        int flag = extractInt("flag");
        for (const Move& m : gen.legalMoves) {
            // Compare against DTOs for readable from/to
            // Build on the fly to avoid another map
            char ff = static_cast<char>('a' + (7 - (m.getSource() % 8)));
            char fr = static_cast<char>('1' + (m.getSource() / 8));
            char tf = static_cast<char>('a' + (7 - (m.getTarget() % 8)));
            char tr = static_cast<char>('1' + (m.getTarget() / 8));
            if (from.size() == 2 && to.size() == 2 &&
                from[0] == ff && from[1] == fr &&
                to[0] == tf && to[1] == tr &&
                (flag == 0 || flag == (int)m.getFlag())) {
                chosen = &m; break;
            }
        }
    }

    if (!chosen) {
        return errorResponse("Illegal move");
    }

    engine->board.makeMove(*chosen);

    stringstream response;
    response << "{";
    response << "\"status\": \"success\",";
    response << "\"action\": \"move\",";
    response << "\"board\": " << boardToJson() << ",";
    response << "\"turn\": \"" << (engine->board.turn ? "white" : "black") << "\",";
    response << "\"timestamp\": " << time(nullptr);
    response << "}";

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
    // debug(string("engine best move: ") + moveNotation);

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

string WebInterface::handleGetLegal() {
    debug("handleGetLegal");
    MoveGen gen;
    std::vector<MoveGen::LegalMoveDTO> dtos;
    gen.getLegalMovesDTO(engine->board, dtos);

    // Build grouped JSON: movesByFrom
    std::stringstream json;
    json << "{";
    json << "\"status\": \"success\",";
    json << "\"action\": \"getlegal\",";
    json << "\"turn\": \"" << (engine->board.turn ? "white" : "black") << "\",";
    json << "\"movesByFrom\": {";

    bool firstFrom = true;
    std::string currentFrom;
    // We will accumulate per from in a simple pass; order doesn't matter
    // Emit by scanning all dtos and grouping inline
    std::vector<std::string> fromKeys;
    fromKeys.reserve(dtos.size());
    for (const auto& d : dtos) fromKeys.push_back(d.from);
    std::sort(fromKeys.begin(), fromKeys.end());
    fromKeys.erase(std::unique(fromKeys.begin(), fromKeys.end()), fromKeys.end());

    for (const auto& from : fromKeys) {
        if (!firstFrom) json << ","; else firstFrom = false;
        json << "\"" << from << "\":[";
        bool firstMove = true;
        for (const auto& d : dtos) {
            if (d.from != from) continue;
            if (!firstMove) json << ","; else firstMove = false;
            json << "{\"id\":" << d.id
                 << ",\"from\":\"" << d.from << "\""
                 << ",\"to\":\"" << d.to << "\""
                 << ",\"flag\":" << d.flag << "}";
        }
        json << "]";
    }

    json << "},";
    json << "\"timestamp\": " << time(nullptr);
    json << "}";

    return json.str();
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
    
    return response.str();
}

void WebInterface::writeStateToFile(const string& filename, const stringstream& state) const {
    ofstream file(filename);
    if (file.is_open()) {
        file << state.str() << endl;
        file.close();
    }
}
