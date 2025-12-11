#include "uci.h"
#include "utility.h"
#include "chess/moveGeneration/moveGen.h"
#include <sstream>
#include <fstream>
#include <streambuf>

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;

UCI::UCI() {
    Board b;
    b.setFEN(STARTING_FEN);
    engine = new Engine(b);
    
    // Set up UCI info callback
    engine->setUciInfoCallback([this](int depth, int timeMs, int nodes, int nps, int scoreCp, const std::vector<Move>& pv) {
        // Temporarily restore cout for UCI info
        std::cout.rdbuf(orig_cout);
        
        // Format UCI info line
        cout << "info depth " << depth 
             << " time " << timeMs 
             << " nodes " << nodes 
             << " nps " << nps;
        
        // UCI mate reporting: score mate N, where N is in MOVES (not plies),
        // from side-to-move perspective (positive = mate for STM, negative = STM is mated)
        const int M = MATE;
        if (scoreCp >= M - 100) {
            int plies = M - scoreCp;              // internal is mate-in-plies
            int moves = (plies + 1) / 2;          // convert to moves
            cout << " score mate " << moves;
        } else if (scoreCp <= -M + 100) {
            int plies = M + scoreCp;              // scoreCp is negative
            int moves = (plies + 1) / 2;
            cout << " score mate " << -moves;    // negative: STM is mated
        } else {
            cout << " score cp " << scoreCp;
        }
        
        // Add principal variation if available
        if (!pv.empty()) {
            cout << " pv";
            for (const Move& move : pv) {
                cout << " " << moveToUci(move);
            }
        }
        
        cout << endl;
        cout.flush();
        
        // Restore null buffer
        std::cout.rdbuf(&nullBuffer);
    });
}

UCI::~UCI() {
    delete engine;
}

static inline string idName() {
    return string("id name Stickshark") + "\n";
}

static inline string idAuthor() {
    return string("id author JaydenShi") + "\n";
}

void UCI::loop() {
    std::ios::sync_with_stdio(false);
    
    // Silence all stdout/stderr during UCI mode using a more robust method
    orig_cout = std::cout.rdbuf();
    
    std::cout.rdbuf(&nullBuffer);
    std::cerr.rdbuf(&nullBuffer);
    
    string line;
    while (std::getline(cin, line)) {
        if (line == "uci") {
            // Temporarily restore cout for UCI responses
            std::cout.rdbuf(orig_cout);
            cout << idName();
            cout << idAuthor();
            cout << "uciok\n";
            cout.flush();
            std::cout.rdbuf(&nullBuffer);
        } else if (line == "isready") {
            std::cout.rdbuf(orig_cout);
            cout << "readyok\n";
            cout.flush();
            std::cout.rdbuf(&nullBuffer);
        } else if (line.rfind("position", 0) == 0) {
            handlePosition(line);
        } else if (line.rfind("go", 0) == 0) {
            handleGo(line);
        } else if (line == "ucinewgame") {
            Board b; b.setFEN(STARTING_FEN); engine->resetEngine(b);
        } else if (line == "stop") {
            // Current engine uses time-limited search only; nothing to cancel here
        } else if (line == "quit") {
            break;
        } else if (line.empty()) {
            continue;
        } else {
            // Ignore unknown commands for now
        }
    }
}

void UCI::handlePosition(const string& line) {
    // Expected forms:
    // position startpos [moves m1 m2 ...]
    // position fen <fen-string> [moves m1 m2 ...]
    size_t p = line.find(' ');
    if (p == string::npos) return;
    size_t q = line.find(' ', p + 1);
    string what = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));

    // Set the initial position on the existing engine board
    if (what == "startpos") {
        engine->board.setFEN(STARTING_FEN);
        p = q == string::npos ? string::npos : line.find("moves", q + 1);
    } else if (what == "fen") {
        // Extract FEN tokens until either end or "moves"
        size_t fenStart = q + 1;
        size_t movesPos = line.find(" moves", fenStart);
        string fen = movesPos == string::npos ? line.substr(fenStart) : line.substr(fenStart, movesPos - fenStart);
        // Trim
        while (!fen.empty() && fen.front() == ' ') fen.erase(fen.begin());
        while (!fen.empty() && fen.back() == ' ') fen.pop_back();
        engine->board.setFEN(fen);
        p = movesPos;
    } else {
        return;
    }
    
    // Reset search stats (TT not cleared)
    engine->resetSearchStats();

    if (p != string::npos) {
        size_t movesStart = line.find(' ', p + 1);
        if (movesStart != string::npos) {
            string rest = line.substr(movesStart + 1);
            // Tokenize on spaces
            size_t i = 0;
            while (i < rest.size()) {
                while (i < rest.size() && rest[i] == ' ') i++;
                size_t j = i;
                while (j < rest.size() && rest[j] != ' ') j++;
                if (j > i) {
                    string token = rest.substr(i, j - i);
                    int src = -1, dst = -1, promoFlag = 0;
                    if (parseUciMoveToken(token, src, dst, promoFlag)) {
                        Move m;
                        if (findLegalMoveBySquares(engine->board, src, dst, promoFlag, m)) {
                            engine->board.makeMove(m);
                        } else {
                            // Ignore illegal token in the sequence
                        }
                    }
                }
                i = j + 1;
            }
        }
    }
}

void UCI::handleGo(const string& line) {
    // Parse UCI time controls: go wtime WT btime BT movestogo MT winc WI binc BI movetime MT
    // Priority: movetime > time per move from wtime/btime/movestogo > default
    
    int movetime = 1000; // default 1s
    
    // Check for explicit movetime first (highest priority)
    size_t mt = line.find("movetime");
    if (mt != string::npos) {
        size_t p = line.find(' ', mt + 8);
        if (p != string::npos) {
            size_t q = line.find(' ', p + 1);
            string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
            try { movetime = std::stoi(num); } catch (...) {}
        }
    } else {
        // Parse time control parameters
        int wtime = -1, btime = -1, movestogo = -1, winc = 0, binc = 0;
        
        // Extract wtime
        size_t wt = line.find("wtime");
        if (wt != string::npos) {
            size_t p = line.find(' ', wt + 5);
            if (p != string::npos) {
                size_t q = line.find(' ', p + 1);
                string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
                try { wtime = std::stoi(num); } catch (...) {}
            }
        }
        
        // Extract btime
        size_t bt = line.find("btime");
        if (bt != string::npos) {
            size_t p = line.find(' ', bt + 5);
            if (p != string::npos) {
                size_t q = line.find(' ', p + 1);
                string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
                try { btime = std::stoi(num); } catch (...) {}
            }
        }
        
        // Extract movestogo
        size_t mtg = line.find("movestogo");
        if (mtg != string::npos) {
            size_t p = line.find(' ', mtg + 9);
            if (p != string::npos) {
                size_t q = line.find(' ', p + 1);
                string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
                try { movestogo = std::stoi(num); } catch (...) {}
            }
        }
        
        // Extract winc and binc (increments)
        size_t wi = line.find("winc");
        if (wi != string::npos) {
            size_t p = line.find(' ', wi + 4);
            if (p != string::npos) {
                size_t q = line.find(' ', p + 1);
                string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
                try { winc = std::stoi(num); } catch (...) {}
            }
        }
        
        size_t bi = line.find("binc");
        if (bi != string::npos) {
            size_t p = line.find(' ', bi + 4);
            if (p != string::npos) {
                size_t q = line.find(' ', p + 1);
                string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
                try { binc = std::stoi(num); } catch (...) {}
            }
        }
        
        // Calculate time per move based on current side to move
        if (wtime > 0 && btime > 0) {
            int currentTime = engine->board.turn ? wtime : btime;
            int increment = engine->board.turn ? winc : binc;
            
            if (movestogo > 0) {
                // Time control with moves remaining
                // Allocate time proportionally, leaving some buffer
                int timePerMove = (currentTime + increment) / movestogo;
                // Apply a more conservative safety factor (use 70% of calculated time)
                // This gives more buffer for complex positions
                movetime = (timePerMove * 70) / 100;
            } else {
                // Sudden death time control (no movestogo specified)
                // Use a smaller portion of remaining time (e.g., 1/30th)
                int timePerMove = currentTime / 30;
                movetime = timePerMove + increment;
            }
        }
    }
    
    // Temporarily restore cout for debug info
    std::cout.rdbuf(orig_cout);
    cout << "info string Time per move: " << movetime << "ms\n";
    cout.flush();
    std::cout.rdbuf(&nullBuffer);
    
    // Engine output is already silenced by the loop, so just call findBestMove
    engine->findBestMove(movetime);
    
    // Temporarily restore cout for bestmove response
    std::cout.rdbuf(orig_cout);
    cout << "bestmove " << moveToUci(engine->bestMove) << "\n";
    cout.flush();
    std::cout.rdbuf(&nullBuffer);
}

string UCI::moveToUci(const Move& m) {
    int s = m.getSource();
    int t = m.getTarget();
    // Flip file coordinates: a->h, b->g, ..., h->a
    char sf = 'h' - (s % 8);  // Flip file: 0->h, 1->g, ..., 7->a
    char sr = '1' + (s / 8);
    char tf = 'h' - (t % 8);  // Flip file: 0->h, 1->g, ..., 7->a
    char tr = '1' + (t / 8);
    string u;
    u += sf; u += sr; u += tf; u += tr;
    // Promotion piece if any
    switch (m.getFlag()) {
        case PROMOTEQUEEN: u += 'q'; break;
        case PROMOTEROOK: u += 'r'; break;
        case PROMOTEBISHOP: u += 'b'; break;
        case PROMOTEKNIGHT: u += 'n'; break;
        default: break;
    }
    return u;
}

bool UCI::parseUciMoveToken(const string& token, int& src, int& dst, int& promoFlag) {
    // token like e2e4 or e7e8q
    if (token.size() < 4) return false;
    char f1 = token[0], r1 = token[1], f2 = token[2], r2 = token[3];
    if (f1 < 'a' || f1 > 'h' || f2 < 'a' || f2 > 'h' || r1 < '1' || r1 > '8' || r2 < '1' || r2 > '8') return false;
    int file1 = 'h' - f1;  // Flip file: 'a'->7, 'b'->6, ..., 'h'->0
    int rank1 = r1 - '1';
    int file2 = 'h' - f2;  // Flip file: 'a'->7, 'b'->6, ..., 'h'->0
    int rank2 = r2 - '1';
    src = rank1 * 8 + file1;
    dst = rank2 * 8 + file2;
    promoFlag = 0;
    if (token.size() >= 5) {
        char p = token[4];
        if (p == 'q') promoFlag = PROMOTEQUEEN;
        else if (p == 'r') promoFlag = PROMOTEROOK;
        else if (p == 'b') promoFlag = PROMOTEBISHOP;
        else if (p == 'n') promoFlag = PROMOTEKNIGHT;
    }
    return true;
}

bool UCI::findLegalMoveBySquares(const Board& board, int src, int dst, int promoFlag, Move& out) {
    MoveGen& gen = MoveGen::getInstance();
    Board copy = board; // generate on a copy so we don't disturb state
    MoveList pseudoMoves = gen.generatePseudoMoves(copy, false);
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &m = pseudoMoves.moves[i];
        if ((int)m.getSource() != src || (int)m.getTarget() != dst) continue;
        // Filter by promotion flag if needed
        if (promoFlag != 0 && (int)m.getFlag() != promoFlag) continue;
        // legality
        copy.makeMove(m);
        bool legal = !copy.kingInCheck(false);
        copy.unmakeMove(m);
        if (legal) { out = m; return true; }
    }
    gen.freePseudoMoves(pseudoMoves);
    return false;
}


