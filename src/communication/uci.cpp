#include "uci.h"
#include "utility.h"
#include "chess/moveGeneration/moveGen.h"
#include "version.h"
#include <sstream>
#include <fstream>
#include <streambuf>

using std::string;
using std::vector;
using std::cout;
using std::cin;
using std::endl;

UCI::UCI(bool useMcts) : book("data/Perfect2023.bin") {
    Board b;
    b.setFEN(STARTING_FEN);
    usingMcts = useMcts;
    agent = usingMcts ? (Agent*) new MCTS(b) : (Agent*) new Engine(b);
    attachInfoCallback();
}

UCI::~UCI() {
    delete agent;
}

// Swap the active agent implementation, carrying the current position over.
void UCI::setAgentType(bool mcts) {
    if (mcts == usingMcts) return;
    Board b = agent->board;
    delete agent;
    agent = mcts ? (Agent*) new MCTS(b) : (Agent*) new Engine(b);
    usingMcts = mcts;
    attachInfoCallback();
}

void UCI::attachInfoCallback() {
    agent->setUciInfoCallback([this](int depth, int timeMs, int nodes, int nps, int scoreCp, const std::vector<Move>& pv) {
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

    agent->setInfoStringCallback([this](const std::string& s) {
        std::cout.rdbuf(orig_cout);
        cout << "info string " << s << "\n";
        cout.flush();
        std::cout.rdbuf(&nullBuffer);
    });
}

static inline string idName() {
    return string("id name Stickshark ") + STICKSHARK_VERSION + "\n";
}

static inline string idAuthor() {
    return string("id author JaydenShi") + "\n";
}

void UCI::loop() {
    std::ios::sync_with_stdio(false);
    
    // Silence all stdout/stderr during UCI mode using a more robust method
    orig_cout = std::cout.rdbuf();
    orig_cerr = std::cerr.rdbuf();

    std::cout.rdbuf(&nullBuffer);
    std::cerr.rdbuf(&nullBuffer);
    
    string line;
    while (std::getline(cin, line)) {
        if (line == "uci") {
            // Temporarily restore cout for UCI responses
            std::cout.rdbuf(orig_cout);
            cout << idName();
            cout << idAuthor();
            cout << "option name SearchAlgorithm type combo default AlphaBeta var AlphaBeta var MCTS\n";
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
        } else if (line.rfind("setoption", 0) == 0) {
            handleSetOption(line);
        } else if (line == "ucinewgame") {
            Board b; b.setFEN(STARTING_FEN); agent->reset(b);
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

    // Restore the real stream buffers: nullBuffer dies with this object, and
    // cout flushes at static teardown — leaving it pointed here segfaults
    std::cout.rdbuf(orig_cout);
    std::cerr.rdbuf(orig_cerr);
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
        agent->board.setFEN(STARTING_FEN);
        p = q == string::npos ? string::npos : line.find("moves", q + 1);
    } else if (what == "fen") {
        // Extract FEN tokens until either end or "moves"
        size_t fenStart = q + 1;
        size_t movesPos = line.find(" moves", fenStart);
        string fen = movesPos == string::npos ? line.substr(fenStart) : line.substr(fenStart, movesPos - fenStart);
        // Trim
        while (!fen.empty() && fen.front() == ' ') fen.erase(fen.begin());
        while (!fen.empty() && fen.back() == ' ') fen.pop_back();
        agent->board.setFEN(fen);
        p = movesPos;
    } else {
        return;
    }
    
    // Reset search stats (TT not cleared)
    agent->resetSearchStats();

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
                        if (findLegalMoveBySquares(agent->board, src, dst, promoFlag, m)) {
                            agent->board.makeMove(m);
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
    int softLimit = 1000;
    int hardLimit = 1000;

    auto parseToken = [&](const string& key, int defaultVal) -> int {
        size_t pos = line.find(key);
        if (pos == string::npos) return defaultVal;
        size_t p = line.find(' ', pos + key.size());
        if (p == string::npos) return defaultVal;
        size_t q = line.find(' ', p + 1);
        string num = q == string::npos ? line.substr(p + 1) : line.substr(p + 1, q - (p + 1));
        try { return std::stoi(num); } catch (...) { return defaultVal; }
    };

    int movetime = parseToken("movetime", -1);

    if (movetime > 0) {
        softLimit = hardLimit = movetime;
    } else {
        int wtime = parseToken("wtime", -1);
        int btime = parseToken("btime", -1);
        int winc  = parseToken("winc",   0);
        int binc  = parseToken("binc",   0);
        int mtg   = parseToken("movestogo", -1);

        if (wtime > 0 && btime > 0) {
            int remaining = std::max((agent->board.turn ? wtime : btime) - 50, 0);
            int increment = agent->board.turn ? winc : binc;

            int movesToGo = (mtg > 0) ? mtg : std::max(40 - agent->board.ply / 2, 20);

            softLimit = (remaining * 6 / 10) / movesToGo + (increment * 8 / 10);
            softLimit = std::min(softLimit, remaining / 5);
            softLimit = std::max(softLimit, 50);

            hardLimit = softLimit * 3;
            hardLimit = std::min(hardLimit, remaining / 8 + (increment * 8 / 10));
            if (mtg > 0) {
                hardLimit = std::min(hardLimit, remaining / movesToGo);
            }
            hardLimit = std::max(hardLimit, softLimit);
            hardLimit = std::max(hardLimit, 50);
        }
    }

    // Try opening book first (alpha-beta only; MCTS should always search)
    if (!usingMcts) {
        auto bookMove = book.probe(agent->board);
        if (bookMove.has_value()) {
            auto [src, dst, promoFlag] = *bookMove;
            Move m;
            if (findLegalMoveBySquares(agent->board, src, dst, promoFlag, m)) {
                std::cout.rdbuf(orig_cout);
                cout << "info string book move\n";
                cout << "bestmove " << moveToUci(m) << "\n";
                cout.flush();
                std::cout.rdbuf(&nullBuffer);
                return;
            }
        }
    }

    // Temporarily restore cout for debug info
    std::cout.rdbuf(orig_cout);
    cout << "info string soft=" << softLimit << "ms hard=" << hardLimit << "ms\n";
    cout.flush();
    std::cout.rdbuf(&nullBuffer);

    agent->findBestMove(softLimit, hardLimit);

    // Temporarily restore cout for bestmove response
    std::cout.rdbuf(orig_cout);
    cout << "bestmove " << moveToUci(agent->bestMove) << "\n";
    cout.flush();
    std::cout.rdbuf(&nullBuffer);
}

void UCI::handleSetOption(const string& line) {
    // Expected form: setoption name <name> value <value>
    size_t namePos = line.find("name ");
    size_t valuePos = line.find(" value ");
    if (namePos == string::npos || valuePos == string::npos) return;

    string name = line.substr(namePos + 5, valuePos - (namePos + 5));
    string value = line.substr(valuePos + 7);

    // Trim
    while (!name.empty() && name.back() == ' ') name.pop_back();
    while (!value.empty() && value.front() == ' ') value.erase(value.begin());
    while (!value.empty() && value.back() == ' ') value.pop_back();

    if (name == "SearchAlgorithm") {
        if (value == "MCTS") {
            setAgentType(true);
        } else if (value == "AlphaBeta") {
            setAgentType(false);
        }

        std::cout.rdbuf(orig_cout);
        cout << "info string search algorithm set to " << (usingMcts ? "MCTS" : "AlphaBeta") << "\n";
        cout.flush();
        std::cout.rdbuf(&nullBuffer);
    }
}

string UCI::moveToUci(const Move& m) {
    return m.toUci();
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
    bool found = false;
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &m = pseudoMoves.moves[i];
        if ((int)m.getSource() != src || (int)m.getTarget() != dst) continue;
        // Filter by promotion flag if needed
        if (promoFlag != 0 && (int)m.getFlag() != promoFlag) continue;
        // legality
        copy.makeMove(m);
        bool legal = !copy.kingInCheck(false);
        copy.unmakeMove(m);
        if (legal) { 
            out = m; 
            found = true;
            break;
        }
    }
    gen.freePseudoMoves(pseudoMoves);
    return found;
}


