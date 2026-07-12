#include "main.h"
#include "communication/uci.h"
#include "benchmark.h"
#include "selfplay/selfplay.h"
#include <sstream>

using namespace std;

int main (int argc, char* argv[]) {
    // Mode flags
    bool uciMode = true;        // UCI protocol on stdin/stdout (default)
    bool benchmarkMode = false; // benchmark mode
    bool perftMode = false;     // debug perft mode
    bool useMcts = false;       // start UCI with the MCTS agent instead of alpha-beta
    int benchmarkDepth = 0;

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--mcts") {
            useMcts = true;
        }
    }

    bool selfplayMode = false;

    if (argc > 1 && string(argv[1]) == "--uci") {
        uciMode = true;
    } else if (argc > 1 && string(argv[1]) == "--selfplay") {
        uciMode = false;
        selfplayMode = true;
    } else if (argc > 1 && string(argv[1]) == "--perft") {
        uciMode = false;
        perftMode = true;
    } else if (argc > 1 && string(argv[1]) == "--benchmark") {
        benchmarkMode = true;
        uciMode = false;
        if (argc > 2) {
            benchmarkDepth = atoi(argv[2]);
        }
    }

    // Suppress initialization output in uciMode
    if (uciMode) {
        streambuf* orig_cout = cout.rdbuf();
        streambuf* orig_cerr = cerr.rdbuf();
        stringstream devnull;
        cout.rdbuf(devnull.rdbuf());
        cerr.rdbuf(devnull.rdbuf());
        computeAllTables();
        cout.rdbuf(orig_cout);
        cerr.rdbuf(orig_cerr);
    } else {
        computeAllTables();
    }

    if (selfplayMode) {
        // usage: --selfplay [games] [outFile] [seed] [moveTimeMs]
        SelfPlayConfig cfg;
        if (argc > 2) cfg.games = atoi(argv[2]);
        if (argc > 3) cfg.outPath = argv[3];
        if (argc > 4) cfg.seed = strtoull(argv[4], nullptr, 10);
        if (argc > 5) cfg.moveTimeMs = atoi(argv[5]);
        runSelfPlay(cfg);
    } else if (benchmarkMode) {
        runBenchmark(benchmarkDepth);
    } else if (uciMode) {
        UCI uci(useMcts);
        uci.loop();
    } else if (perftMode) {
        perft(5, STARTING_FEN);
    }

    return 0;
}
