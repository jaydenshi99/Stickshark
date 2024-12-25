#include "utility.h"

using namespace std;

void displayBitboard(uint64_t bb) {
    for (int i = 7; i >= 0; --i) {
        for (int j = 7; j >= 0; --j) {
            int bit = (bb >> (i * 8 + j)) & 1ULL;

            if (bit == 1) {
                cout << "#";
            } else {
                cout << ".";
            }

            cout << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void simulateRandomMoves() {
    Board board;
    MoveGen moveGen;

    cout << "Starting Position: " << endl;
    board.setStartingPosition();
    board.displayBoard();

    moveGen.generatePseudoMoves(board);

    int move = 1;
    while (moveGen.moves.size() != 0 && move < 10) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, moveGen.moves.size() - 1);

        int randIndex = dist(gen);

        board.makeMove(moveGen.moves[randIndex]);

        cout << move++ << ". ";
        if (board.turn) {
            cout << "White to move";
        } else {
            cout << "Black to move";
        }
        cout << endl;

        board.displayBoard();

        moveGen.clearMoves();
        moveGen.generatePseudoMoves(board);
    }
}

void displayPossibleMoves() {
    Board board;
    MoveGen moveGen;

    board.setStartingPosition();
    board.displayBoard();

    moveGen.generatePseudoMoves(board);

    for (Move move : moveGen.moves) {
        board.makeMove(move);
        board.displayBoard();
        board.unmakeMove(move);
    }

    cout << "Total Moves: " << moveGen.moves.size() << "\n\n";
}

void perft(int depth) {
    Board board;

    board.setStartingPosition();

    auto start = chrono::high_resolution_clock::now();

    long totalMoves = perftRecursive(board, depth);

    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;

    cout << "Total Moves: " << totalMoves << endl;

    double movesPerSecond = totalMoves / (static_cast<double>(duration.count()) / 1000);

    cout << fixed << setprecision(0);
    cout << "Moves / Second: " << movesPerSecond << endl;
}

long perftRecursive(Board& b, int depth) {
    if (depth == 0) {
        return 1;
    }

    MoveGen mg;
    mg.generatePseudoMoves(b);

    long totalMoves = 0;
    for (Move m : mg.moves) {
        b.makeMove(m);
        totalMoves += perftRecursive(b, depth - 1);
        b.unmakeMove(m);
    }

    return totalMoves;
}