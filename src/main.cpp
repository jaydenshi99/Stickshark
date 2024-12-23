#include "main.h"

using namespace std;

int main () {
    computeAllTables();

    Board board;
    MoveGen moveGen;

    board.setStartingPosition();
    board.displayBoard();

    // board.swapTurn();
    // moveGen.generatePawnMoves(board);

    // for (Move move : moveGen.moves) {
    //     board.makeMove(move);
    //     board.displayBoard();
    //     board.unmakeMove(move);
    // }

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
    
    return 0;
}