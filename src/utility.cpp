#include "utility.h"
#include <cassert>
#include <chrono>
#include <iomanip>

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
    MoveGen& moveGen = MoveGen::getInstance();

    cout << "Starting Position: " << endl;
    board.setStartingPosition();
    board.displayBoard();

    MoveList pseudoMoves = moveGen.generatePseudoMoves(board, false);

    int move = 1;
    while (pseudoMoves.count != 0 && move <= 1) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, pseudoMoves.count - 1);

        int randIndex = dist(gen);

        board.makeMove(pseudoMoves.moves[randIndex]);

        cout << move++ << ". ";
        if (board.turn) {
            cout << "White to move";
        } else {
            cout << "Black to move";
        }
        cout << endl;

        board.displayBoard();

        moveGen.freePseudoMoves(pseudoMoves);
        pseudoMoves = moveGen.generatePseudoMoves(board, false);
    }

    moveGen.freePseudoMoves(pseudoMoves);

    for (int i = 0; i < 12; i++) {
        cout << i << endl;
        displayBitboard(board.history.top().attackBitboards[i]);
    }
}

void displayPossibleMoves(string FEN) {
    Board board;
    MoveGen& moveGen = MoveGen::getInstance();

    board.setFEN(FEN);
    board.displayBoard();

    MoveList pseudoMoves = moveGen.generatePseudoMoves(board, false);
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &move = pseudoMoves.moves[i];
        board.makeMove(move);
        board.displayBoard();
        board.unmakeMove(move);
    }

    cout << "Total Moves: " << pseudoMoves.count << "\n\n";

    moveGen.freePseudoMoves(pseudoMoves);
}

long perft(int depth, string FEN, bool debug) {
    if (debug) {
        cout << "Performance testing move generation for board: " << endl;
    }

    Board board;
    board.setFEN(FEN);

    if (debug) {
        board.displayBoard();

        cout << "Zobrist before: " << board.zobristHash << endl;
    
        cout << "Searching to depth " << depth << "..." << endl;
    }

    auto start = chrono::high_resolution_clock::now();

    long totalMoves = perftRecursive(board, depth);

    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    if (debug) {
        cout << "Time taken: " << duration.count() << " ms" << endl;

        cout << "Total Moves: " << totalMoves << endl;
    }

    double movesPerSecond = totalMoves / (static_cast<double>(duration.count()) / 1000);

    if (debug) {
        cout << fixed << setprecision(0);
        cout << "Moves / Second: " << movesPerSecond << endl;

        cout << "Zobrist after: " << board.zobristHash << endl;
    }

    return totalMoves;
}

long perftRecursive(Board& b, int depth) {
    MoveGen& mg = MoveGen::getInstance();
    MoveList pseudoMoves = mg.generatePseudoMoves(b, false);

    long totalMoves = 0;
    for (std::ptrdiff_t i = 0; i < pseudoMoves.count; i++) {
        Move &m = pseudoMoves.moves[i];
        b.makeMove(m);
        if (!b.kingInCheck(false)) {
            totalMoves += depth == 1 ? 1 : perftRecursive(b, depth - 1);
        }
        b.unmakeMove(m);
    }

    mg.freePseudoMoves(pseudoMoves);

    return totalMoves;
}

void playEngine(string startingFEN, int time) {
    Board board = Board();
    board.setFEN(startingFEN);
    Engine engine = Engine(board);

    char playerColour;
    cout << "Player is white or black? (w/b) ";
    cin >> playerColour;

    if (playerColour != 'w' && playerColour != 'b') {
        cout << "Invalid input" << endl;
        return;
    }

    bool playerTurn = playerColour == 'w';

    int moveNum = 1;
    cout << endl << moveNum << "." << endl;
    engine.board.displayBoard();

    // Get terminating position after implementing checkmates
    while (true) {
        if (playerTurn) {
            string playerMove; 
            if (moveNum <= 2) cout << "Please input move in format 'squareFrom-squareTo', eg. e2-e4" << endl;
            cout << "Move: ";

            cin >> playerMove;
            cout << endl;

            engine.board.makeMove(notationToMove(playerMove, engine.board.turn));
        } else {
            engine.findBestMove(time);
            engine.board.makeMove(engine.bestMove);
        }

        moveNum++;
        cout << moveNum << "." << endl;
        engine.board.displayBoard();

        playerTurn = !playerTurn;
    }
}

void engineVSEngine(string startingFEN, int time) {
    Board board = Board();
    board.setFEN(startingFEN);

    int moveNum = 1;
    Engine engine = Engine(board);
    while (true) {
        cout << moveNum++ << "." << endl;
        engine.board.displayBoard();

        engine.findBestMove(time);
        engine.board.makeMove(engine.bestMove);
    }
}

void runEngineToDepth(string FEN, int depth) {
    cout << "Creating engine with position: " << FEN << endl;

    // Create board and engine
    Board board;
    board.setFEN(FEN);

    cout << "Initial board position:" << endl;
    board.displayBoard();

    Engine engine(board);

    cout << "\nSearching to depth " << depth << "..." << endl;
    cout << "================================================" << endl;

    // Use findBestMove with a very large time limit and specific max depth
    engine.findBestMove(600000, depth);  // 10 minute time limit, but will stop at depth

    cout << "================================================" << endl;
}

// Only covers normal moves no special moves
Move notationToMove(string move, bool turn) {
    // Castling
    if (move == "O-O") {
        // king side
        const int kingFrom = turn ? WK_START_SQUARE : BK_START_SQUARE;
        const int kingTo   = turn ? 1 : 57;
        return Move(kingFrom, kingTo, CASTLING);
    } else if (move == "O-O-O") {
        // queen side
        const int kingFrom = turn ? WK_START_SQUARE : BK_START_SQUARE;
        const int kingTo   = turn ? 5 : 61;
        return Move(kingFrom, kingTo, CASTLING);
    }

    // Coordinate moves
    int sourceSquare = 'h' - move[0] + (move[1] - '1') * 8;
    int targetSquare = 'h' - move[3] + (move[4] - '1') * 8;
    return Move(sourceSquare, targetSquare, NONE);
}