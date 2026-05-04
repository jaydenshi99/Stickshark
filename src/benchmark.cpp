#include "benchmark.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;

void runBenchmark(int customDepth) {
    struct TestPosition {
        string name;
        string fen;
        int depth;
        string description;
    };

    vector<TestPosition> testPositions = {
        {
            "Open Italian Game",
            "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
            8,
            "Italian Game after 3.Bc4 - open center"
        },
        {
            "Sicilian Defense Middlegame",
            "r1bqkb1r/1p3ppp/p1np1n2/4p3/2BPP3/2N2N2/PPP2PPP/R1BQK2R w KQkq - 0 8",
            8,
            "Sicilian Najdorf - complex middlegame"
        },
        {
            "Queen's Gambit Middlegame",
            "r1bq1rk1/ppp1bppp/2n2n2/3p4/2PP4/2N1PN2/PP3PPP/R1BQKB1R w KQ - 0 8",
            8,
            "QGD - balanced middlegame"
        },
        {
            "Tactical Middlegame",
            "r1bq1rk1/2p2ppp/pb2pn2/1p1p4/2nP1B2/2PBPN2/PPQN1PPP/R4RK1 w - - 10 8",
            8,
            "Complex middlegame with tactical possibilities"
        },
        {
            "Complicated Middlegame",
            COMPLICATED_MIDDLEGAME,
            8,
            "Sharp middlegame position with imbalanced material"
        },
        {
            "Ruy Lopez Middlegame",
            "r2qkb1r/1pp2ppp/p1pb1n2/4p3/3PP3/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 0 8",
            8,
            "Ruy Lopez Berlin - strategic middlegame"
        },
        {
            "French Defense Middlegame",
            "rnbqk2r/ppp2ppp/4pn2/3p4/1b1PP3/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 2 6",
            8,
            "French Defense - closed center middlegame"
        },
        {
            "King's Indian Middlegame",
            "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 0 6",
            8,
            "King's Indian - dynamic middlegame"
        },
        {
            "English Opening Middlegame",
            "r1bqk2r/pp1nbppp/2p1pn2/3p4/2PP4/2N1PN2/PP3PPP/R1BQKB1R w KQkq - 0 8",
            8,
            "English Opening - strategic complexity"
        }
    };

    cout << "========================================" << endl;
    cout << "    ENGINE DEPTH BENCHMARK" << endl;
    cout << "========================================" << endl;

    // Override depths if custom depth provided
    if (customDepth > 0 && customDepth <= 20) {
        cout << "Using custom depth: " << customDepth << endl;
        for (auto& pos : testPositions) {
            pos.depth = customDepth;
        }
    }

    cout << "Testing " << testPositions.size() << " positions\n" << endl;

    int totalTests = testPositions.size();
    int completedTests = 0;
    long long totalTimeMs = 0;

    // Run each test
    for (size_t i = 0; i < testPositions.size(); i++) {
        const TestPosition& test = testPositions[i];

        cout << "\n\n========================================" << endl;
        cout << "TEST " << (i + 1) << " of " << totalTests << ": " << test.name << endl;
        cout << "========================================" << endl;
        cout << "Description: " << test.description << endl;
        cout << "FEN: " << test.fen << endl;
        cout << "Target Depth: " << test.depth << endl;
        cout << endl;

        auto testStart = chrono::high_resolution_clock::now();

        try {
            runEngineToDepth(test.fen, test.depth);
            completedTests++;
        } catch (const exception& e) {
            cout << "ERROR: Test failed with exception: " << e.what() << endl;
        }

        auto testEnd = chrono::high_resolution_clock::now();
        auto testDuration = chrono::duration_cast<chrono::milliseconds>(testEnd - testStart);
        totalTimeMs += testDuration.count();

        cout << "\nTest " << (i + 1) << " completed." << endl;
        cout << "========================================" << endl;
    }

    // Summary
    cout << "\n\n========================================" << endl;
    cout << "    BENCHMARK SUMMARY" << endl;
    cout << "========================================" << endl;
    cout << "Total tests: " << totalTests << endl;
    cout << "Completed: " << completedTests << endl;
    cout << "Failed: " << (totalTests - completedTests) << endl;
    cout << "Total time: " << totalTimeMs << " ms";
    cout << " (" << fixed << setprecision(2) << totalTimeMs / 1000.0 << " seconds)" << endl;
    cout << "Average time per test: " << fixed << setprecision(2)
         << (completedTests > 0 ? totalTimeMs / (double)completedTests : 0) << " ms" << endl;
    cout << "========================================" << endl;
}
