#include "selfplay.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "../engine/engine.h"
#include "../engine/openingBook.h"
#include "../chess/moveGeneration/moveGen.h"

using namespace std;

namespace {

uint64_t rngState = 1;

uint64_t nextRandom() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    return rngState;
}

bool findLegalBySquares(Board& board, int src, int dst, int promoFlag, Move& out) {
    MoveGen& mg = MoveGen::getInstance();
    MoveList pseudo = mg.generatePseudoMoves(board, false);
    bool found = false;
    for (ptrdiff_t i = 0; i < pseudo.count; i++) {
        Move& m = pseudo.moves[i];
        if ((int)m.getSource() != src || (int)m.getTarget() != dst) continue;
        if (promoFlag != 0 && (int)m.getFlag() != promoFlag) continue;
        board.makeMove(m);
        bool legal = !board.kingInCheck(false);
        board.unmakeMove(m);
        if (legal) {
            out = m;
            found = true;
            break;
        }
    }
    mg.freePseudoMoves(pseudo);
    return found;
}

// returns false if no legal move exists
bool playRandomLegal(Board& board) {
    MoveGen& mg = MoveGen::getInstance();
    MoveList legal = mg.generateLegalMoves(board);
    if (legal.count == 0) {
        mg.freeLegalMoves(legal);
        return false;
    }
    Move m = legal.moves[nextRandom() % legal.count];
    mg.freeLegalMoves(legal);
    board.makeMove(m);
    return true;
}

struct PendingRecord {
    TrainingRecord rec;
    bool whiteToMove;
    uint64_t hash;
};

}

void runSelfPlay(const SelfPlayConfig& cfg) {
    // splitmix64 scramble so consecutive worker seeds give independent streams
    uint64_t z = (cfg.seed ? cfg.seed : 1) + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    rngState = z ^ (z >> 31);
    if (!rngState) rngState = 1;

    MoveGen& mg = MoveGen::getInstance();
    OpeningBook book("data/Perfect2023.bin");

    Board start;
    start.setFEN(STARTING_FEN);
    Engine engine(start);

    ofstream out(cfg.outPath, ios::binary);
    if (!out) {
        cout << "selfplay: cannot open " << cfg.outPath << endl;
        return;
    }
    uint32_t magic = 0x50535353;
    int32_t version = 1;
    out.write(reinterpret_cast<const char*>(&magic), 4);
    out.write(reinterpret_cast<const char*>(&version), 4);

    unordered_set<uint64_t> seen;
    long totalPositions = 0;
    long totalDuplicates = 0;
    auto t0 = chrono::steady_clock::now();

    vector<PendingRecord> gameRecords;

    for (int g = 0; g < cfg.games; g++) {
        Board board;
        board.setFEN(STARTING_FEN);
        gameRecords.clear();

        // opening book walk to a random depth, sometimes zero
        int bookLimit = (int)(nextRandom() % (uint64_t)(cfg.maxBookPlies + 1));
        for (int i = 0; i < bookLimit; i++) {
            auto bm = book.probe(board);
            if (!bm) break;
            auto [src, dst, promo] = *bm;
            Move m;
            if (!findLegalBySquares(board, src, dst, promo, m)) break;
            board.makeMove(m);
        }

        // random plies to guarantee a unique start
        int span = cfg.maxRandomPlies - cfg.minRandomPlies + 1;
        int randomPlies = cfg.minRandomPlies + (int)(nextRandom() % (uint64_t)span);
        bool aborted = false;
        for (int i = 0; i < randomPlies; i++) {
            if (!playRandomLegal(board)) {
                aborted = true;
                break;
            }
        }
        if (aborted) continue;   // random phase ended the game; position is junk

        int result = 0;   // white POV
        bool decided = false;
        bool fullPlayout = (nextRandom() % 1000) < (uint64_t)(cfg.playoutFraction * 1000);
        int adjCount = 0;
        int adjSide = 0;
        int16_t lastScore = 0;
        bool lastScoreWhite = true;

        for (int ply = 0; ply < cfg.maxPlies && !decided; ply++) {
            MoveList legal = mg.generateLegalMoves(board);
            ptrdiff_t nLegal = legal.count;
            mg.freeLegalMoves(legal);
            if (nLegal == 0) {
                result = board.kingInCheck(true) ? (board.turn ? -1 : 1) : 0;
                decided = true;
                break;
            }
            if (board.isThreeFoldRepetition()) {
                result = 0;
                decided = true;
                break;
            }

            bool noise = ply < cfg.noiseCutoffPly &&
                         (nextRandom() % 1000) < (uint64_t)(cfg.epsilonNoise * 1000);
            if (noise) {
                playRandomLegal(board);
                continue;
            }

            // engine move (cout silenced: findBestMove prints debug stats)
            engine.setPosition(board);
            streambuf* orig = cout.rdbuf();
            stringstream devnull;
            cout.rdbuf(devnull.rdbuf());
            engine.findBestMove(cfg.moveTimeMs, cfg.moveTimeMs);
            cout.rdbuf(orig);

            Move best = engine.bestMove;
            int16_t score = engine.boardEval;

            PendingRecord pr;
            nn::encodeCompact(board, pr.rec.pos);
            pr.rec.policyIndex = (int16_t)nn::policyIndex(best, board.turn);
            pr.rec.scoreCp = score;
            pr.rec.result = 0;
            pr.rec.pad = 0;
            pr.whiteToMove = board.turn;
            pr.hash = board.zobristHash;
            gameRecords.push_back(pr);

            lastScore = score;
            lastScoreWhite = board.turn;

            // adjudicate only on consecutive big evals that agree on the winner
            int whiteCp = board.turn ? score : -score;
            int side = whiteCp >= cfg.adjudicateCp ? 1
                     : whiteCp <= -cfg.adjudicateCp ? -1 : 0;
            adjCount = (side != 0 && side == adjSide) ? adjCount + 1 : (side != 0);
            adjSide = side;

            board.makeMove(best);

            if (!fullPlayout && adjCount >= cfg.adjudicateMoves) {
                result = adjSide;
                decided = true;
            }
        }

        if (!decided) {
            int whiteCp = lastScoreWhite ? lastScore : -lastScore;
            result = whiteCp >= 200 ? 1 : (whiteCp <= -200 ? -1 : 0);
        }

        for (auto& pr : gameRecords) {
            if (!seen.insert(pr.hash).second) {
                totalDuplicates++;
                continue;
            }
            pr.rec.result = (int8_t)(pr.whiteToMove ? result : -result);
            out.write(reinterpret_cast<const char*>(&pr.rec), sizeof(TrainingRecord));
            totalPositions++;
        }

        if ((g + 1) % 10 == 0 || g + 1 == cfg.games) {
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - t0).count();
            double rate = elapsed > 0 ? (double)totalPositions / elapsed : 0;
            cout << "selfplay: " << (g + 1) << "/" << cfg.games << " games, "
                 << totalPositions << " positions (" << totalDuplicates << " dup skipped), "
                 << (long)rate << " pos/s" << endl;
        }
    }

    out.close();
    cout << "selfplay: wrote " << totalPositions << " records to " << cfg.outPath << endl;
}
