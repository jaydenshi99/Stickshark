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

    moveGen.generatePawnMoves(board);

    int move = 1;
    while (moveGen.moves.size() != 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, moveGen.moves.size() - 1);

        int randIndex = dist(gen);

        board.makeMove(moveGen.moves[randIndex]);

        cout << move++ << ".\n";
        board.displayBoard();

        moveGen.clearMoves();
        moveGen.generatePawnMoves(board);
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