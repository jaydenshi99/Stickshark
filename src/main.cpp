#include "main.h"

using namespace std;

int main () {
    Board board;

    board.setStartingPosition();
    board.displayBoard();

    vector<Move> pawnMoves = generatePawnMoves(board);

    for (Move m : pawnMoves) {
        board.makeMove(m);
        board.displayBoard();
        board.unmakeMove(m);
    }
    
    return 0;
}