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

    Board b;
    if (what == "startpos") {
        b.setFEN(STARTING_FEN);
        p = q == string::npos ? string::npos : line.find("moves", q + 1);
    } else if (what == "fen") {
        // Extract FEN tokens until either end or "moves"
        size_t fenStart = q + 1;
        size_t movesPos = line.find(" moves", fenStart);
        string fen = movesPos == string::npos ? line.substr(fenStart) : line.substr(fenStart, movesPos - fenStart);
        // Trim
        while (!fen.empty() && fen.front() == ' ') fen.erase(fen.begin());
        while (!fen.empty() && fen.back() == ' ') fen.pop_back();
        b.setFEN(fen);
        p = movesPos;
    } else {
        return;
    }

    engine->resetEngine(b);

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
    // Parse a very small subset: go movetime X  OR go wtime WT btime BT winc WI binc BI (we'll prefer movetime if present)
    int movetime = 1000; // default 1s
    size_t mt = line.find("movetime");
    if (mt != string::npos) {
        size_t p = line.find(' ', mt + 8);
        if (p != string::npos) {
            size_t q = line.find(' ', p + 1);
            string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
            try { movetime = std::stoi(num); } catch (...) {}
        }
    }
    
    // Engine output is already silenced by the loop, so just call findBestMove
    engine->findBestMove(movetime);
    
    // Temporarily restore cout for bestmove response
    std::cout.rdbuf(orig_cout);
    cout << "pv " << moveToUci(engine->bestMove) << "\n";
    cout.flush();
    std::cout.rdbuf(&nullBuffer);
}

string UCI::moveToUci(const Move& m) {
    int s = m.getSource();
    int t = m.getTarget();
    char sf = 'a' + (s % 8);
    char sr = '1' + (s / 8);
    char tf = 'a' + (t % 8);
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
    int file1 = f1 - 'a';
    int rank1 = r1 - '1';
    int file2 = f2 - 'a';
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
    MoveGen gen;
    Board copy = board; // generate on a copy so we don't disturb state
    gen.generatePseudoMoves(copy);
    for (const Move& m : gen.pseudoMoves) {
        if ((int)m.getSource() != src || (int)m.getTarget() != dst) continue;
        // Filter by promotion flag if needed
        if (promoFlag != 0 && (int)m.getFlag() != promoFlag) continue;
        // legality
        copy.makeMove(m);
        bool legal = !copy.kingInCheck();
        copy.unmakeMove(m);
        if (legal) { out = m; return true; }
    }
    return false;
}


