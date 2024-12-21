#include "main.h"

using namespace std;

int main () {
    Board board;
    MoveGen moveGen;

    board.setStartingPosition();
    board.displayBoard();

    moveGen.generatePawnMoves(board);

    for (Move m : moveGen.moves) {
        board.makeMove(m);
        board.displayBoard();
        board.unmakeMove(m);
    }
    
    return 0;
}