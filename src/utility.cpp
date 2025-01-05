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
    while (moveGen.moves.size() != 0 && move <= 1) {
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

    for (int i = 0; i < 12; i++) {
        cout << i << endl;
        displayBitboard(board.history.top().attackBitboards[i]);
    }
}

void displayPossibleMoves(string FEN) {
    Board board;
    MoveGen moveGen;

    board.setFEN(FEN);
    board.displayBoard();

    moveGen.generatePseudoMoves(board);
    cout << moveGen.moves.size() << endl;
    for (Move move : moveGen.moves) {
        board.makeMove(move);
        board.displayBoard();
        board.unmakeMove(move);
    }

    cout << "Total Moves: " << moveGen.moves.size() << "\n\n";
}

void perft(int depth, string FEN) {
    cout << "Performance testing move generation for board: " << endl;

    Board board;
    board.setFEN(FEN);

    board.displayBoard();
    
    cout << "Searching to depth " << depth << "..." << endl;

    auto start = chrono::high_resolution_clock::now();

    long totalMoves = perftRecursive(board, depth);

    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Time taken: " << duration.count() << " ms" << endl;

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
        if (!b.kingInCheck()) {
            totalMoves += perftRecursive(b, depth - 1);
        }
        b.unmakeMove(m);
    }

    return totalMoves;
}

void playAI() {
    Board board = Board();
    board.setStartingPosition();

    char playerColour;
    cout << "Player is white or black? (w/b)";
    cin >> playerColour;

    if (playerColour != 'w' && playerColour != 'b') {
        cout << "Invalid input" << endl;
        return;
    }

    bool playerTurn = playerColour == 'w';

    int moveNum = 1;
    cout << endl << moveNum << "." << endl;
    board.displayBoard();

    // Get terminating position after implementing checkmates n shi
    while (true) {
        if (playerTurn) {
            string playerMove; 
            cout << "Move: ";
            cin >> playerMove;
            cout << endl;

            board.makeMove(notationToMove(playerMove));
        } else {
            Engine engine = Engine(board);
            engine.findBestMove(7);
            board.makeMove(engine.bestMove);
        }

        moveNum++;
        cout << moveNum << "." << endl << endl;
        board.displayBoard();

        playerTurn = !playerTurn;
    }
}

// Only covers normal moves no special moves
Move notationToMove(string move) {
    int sourceSquare = 'h' - move[0] + (move[1] - '1') * 8;
    int targetSquare = 'h' - move[3] + (move[4] - '1') * 8;
    return Move(sourceSquare, targetSquare, NONE);
}