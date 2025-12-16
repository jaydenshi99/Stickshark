#include "testRunner.h"
#include "chess/board/board.h"
#include "constants.h"
#include "chess/moveGeneration/bitTables.h"
#include "utility.h"
#include <string>
#include <vector>
#include <utility>

using namespace std;

void checkPerft(const string& fen, int depth, long expected) {
    long result = perft(depth, fen, false);
    std::cout << "    Depth " << depth << ": " << result << " (expected: " << expected << ")" << std::endl;
    ASSERT_EQ(result, expected);
}

void checkPerftDepths(const string& fen, const vector<long>& expected) {
    cout << fen << endl;

    // Assumes sequential depths starting from 1
    for (size_t i = 0; i < expected.size(); i++) {
        int depth = static_cast<int>(i + 1);
        checkPerft(fen, depth, expected[i]);
    }
}

bool testStartingPerft() {
    checkPerftDepths(STARTING_FEN, {20L, 400L, 8902L, 197281L, 4865609L, 119060324L});
    return true;
}

bool testKiwipetePerft() {
    string FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    checkPerftDepths(FEN, {48L, 2039L, 97862L, 4085603L, 193690690L});
    return true;
}

bool testPawnRooksPerft() {
    string FEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ";
    checkPerftDepths(FEN, {14L, 191L, 2812L, 43238L, 674624L, 11030083L, 178633661L});
    return true;
}

bool testPosition4Perft() {
    string FEN = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    checkPerftDepths(FEN, {6L, 264L, 9467L, 422333L, 15833292L});
    return true;
}

bool testPosition5Perft() {
    string FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    checkPerftDepths(FEN, {44L, 1486L, 62379L, 2103487L, 89941194L});
    return true;
}

bool testPosition6Perft() {
    string FEN = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
    checkPerftDepths(FEN, {46L, 2079L, 89890L, 3894594L, 164075551L});
    return true;
}

int main() {
    computeAllTables();

    TestRunner runner;
    
    std::cout << "Running Move Generation Tests...\n" << std::endl;
    
    runner.run("Perft Starting Position", testStartingPerft);
    runner.run("Perft Kiwipete", testKiwipetePerft);
    runner.run("Perft Pawn Rooks", testPawnRooksPerft);
    runner.run("Perft Position 4", testPosition4Perft);
    runner.run("Perft Position 5", testPosition5Perft);
    runner.run("Perft Position 6", testPosition6Perft);

    runner.printSummary();
    return runner.getExitCode();
}

